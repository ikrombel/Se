#include "ResourceCache.h"

// #include <GFrost/Core/Context.h>
// #include <GFrost/Core/CoreEvents.h>
#include <Se/Profiler.hpp>
#include <Se/WorkQueue.h>
#include <Se/IO/FileSystem.h>
#include <SeVFS/FileWatcher.h>
#include <Se/Console.hpp>
#include <Se/IO/PackageFile.h>
#include <SeVFS/VirtualFileSystem.h>
#include "BackgroundLoader.h"
#include <SeResource/Image.h>
//#include <SeResource/ImageCube.h>
#include <SeResource/JSONFile.h>
//#include "PListFile.h"
//#include "SerializableResource.h"
//#include "ResourceEvents.h"
#include <SeResource/XMLFile.h>
//#include <Se/IO/BinaryFile.h>
#include <SeResource/JSONArchive.h>
//#include <SeResource/XMLArchive.h>

//#include <GFrost/DebugNew.h>

#include <cstdio>
#include <memory>

namespace Se
{

bool NeedToReloadDependencies(Resource* resource)
{
    // It should always return true in perfect world, but I never tested it.
    if (!resource)
        return true;
    const String extension = GetExtension(resource->GetName());
    return extension == ".xml"
        || extension == ".glsl"
        || extension == ".hlsl";
}


static const char* checkDirs[] =
{
    "Fonts",
    "Materials",
    "Models",
    "Music",
    "Objects",
    "Particle",
    "PostProcess",
    "RenderPaths",
    "Scenes",
    "Scripts",
    "Sounds",
    "Shaders",
    "Techniques",
    "Textures",
    "UI",
    nullptr
};

static const std::shared_ptr<Resource> noResource;

std::unordered_map<String, std::function<std::shared_ptr<Resource>()>> ResourceCache::resourceFactory_;
//std::vecto<String, String> ResourceCache::resourceNames_;

ResourceCache::ResourceCache() :
    returnFailedResources_(false),
    searchPackagesFirst_(true),
    finishBackgroundResourcesMs_(5)
{
    // Register Resource library object factories
    //RegisterResourceLibrary(context_);

#ifdef SE_THREADING
    // Create resource background loader. Its thread will start on the first background request
    backgroundLoader_ = std::make_shared<BackgroundLoader>(this);
#endif

    // Subscribe BeginFrame for handling directory watchers and background loaded resource finalization
    //SubscribeToEvent(E_BEGINFRAME, SE_HANDLER(ResourceCache, HandleBeginFrame));
    Time::onBeginFrame.connect([this](const TimeParams&){
        HandleBeginFrame();
    });

    // // Subscribe FileChanged for handling directory watchers
    // SubscribeToEvent(E_FILECHANGED, SE_HANDLER(ResourceCache, HandleFileChanged));

    FileWatcher::onFileChanged.connect([this](const FileChangeInfo& fileInfo){
        HandleFileChanged(fileInfo);
    });

    // // Subscribe to reflection removal to purge unloaded resource types
    // context_->OnReflectionRemoved.Subscribe(this, &ResourceCache::HandleReflectionRemoved);
}

ResourceCache::~ResourceCache()
{
#ifdef SE_THREADING
    // Shut down the background loader first
    backgroundLoader_.reset();
#endif
}

bool ResourceCache::AddManualResource(std::shared_ptr<Resource> resource)
{
    if (!resource)
    {
        SE_LOG_ERROR("Null manual resource");
        return false;
    }

    const String& name = resource->GetName();
    if (name.empty())
    {
        SE_LOG_ERROR("Manual resource with empty name, can not add");
        return false;
    }

    resource->ResetUseTimer();
    resourceGroups_[resource->GetType()].resources_[resource->GetName()] = resource;
    UpdateResourceGroup(resource->GetType());
    return true;
}

void ResourceCache::ReleaseResource(String type, const String& name, bool force)
{
    String nameHash(name);
    const std::shared_ptr<Resource>& existingRes = FindResource(type, nameHash);
    if (!existingRes)
        return;

    // If other references exist, do not release, unless forced
    if ((existingRes.use_count() == 1 && existingRes.unique()) || force)
    {
        resourceGroups_[type].resources_.erase(nameHash);
        UpdateResourceGroup(type);
    }
}

void ResourceCache::ReleaseResource(const String& resourceName, bool force)
{
    // Some resources refer to others, like materials to textures. Repeat the release logic as many times as necessary to ensure
    // these get released. This is not necessary if forcing release
    bool released;
    do
    {
        released = false;

        for (auto i = resourceGroups_.begin(); i != resourceGroups_.end(); ++i)
        {
            for (auto j = i->second.resources_.begin(); j != i->second.resources_.end();)
            {
                auto current = i->second.resources_.find(resourceName);
                if (current != i->second.resources_.end())
                {
                    // If other references exist, do not release, unless forced
                    if ((current->second.use_count() == 1 && current->second.unique()) || force)
                    {
                        j = i->second.resources_.erase(current);
                        released = true;
                        continue;
                    }
                }
                ++j;
            }
            if (released)
                UpdateResourceGroup(i->first);
        }

    } while (released && !force);
}

void ResourceCache::ReleaseResources(String type, bool force)
{
    bool released = false;

    auto i = resourceGroups_.find(type);
    if (i != resourceGroups_.end())
    {
        for (auto j = i->second.resources_.begin();
             j != i->second.resources_.end();)
        {
            auto current = j++;
            // If other references exist, do not release, unless forced
            if ((current->second.use_count() == 1 && current->second.unique()) || force)
            {
                i->second.resources_.erase(current);
                released = true;
            }
        }
    }

    if (released)
        UpdateResourceGroup(type);
}

void ResourceCache::ReleaseResources(String type, const String& partialName, bool force)
{
    bool released = false;

    auto i = resourceGroups_.find(type);
    if (i != resourceGroups_.end())
    {
        for (auto j = i->second.resources_.begin();
             j != i->second.resources_.end();)
        {
            auto current = j++;
            if (current->second->GetName().contains(partialName))
            {
                // If other references exist, do not release, unless forced
                if ((current->second.use_count() == 1 && current->second.unique()) || force)
                {
                    i->second.resources_.erase(current);
                    released = true;
                }
            }
        }
    }

    if (released)
        UpdateResourceGroup(type);
}

void ResourceCache::ReleaseResources(const String& partialName, bool force)
{
    // Some resources refer to others, like materials to textures. Repeat the release logic as many times as necessary to ensure
    // these get released. This is not necessary if forcing release
    bool released;
    do
    {
        released = false;

        for (auto i = resourceGroups_.begin(); i != resourceGroups_.end(); ++i)
        {
            for (auto j = i->second.resources_.begin();
                 j != i->second.resources_.end();)
            {
                auto current = j++;
                if (current->second->GetName().contains(partialName))
                {
                    // If other references exist, do not release, unless forced
                    if ((current->second.use_count() == 1 && current->second.unique()) || force)
                    {
                        i->second.resources_.erase(current);
                        released = true;
                    }
                }
            }
            if (released)
                UpdateResourceGroup(i->first);
        }

    } while (released && !force);
}

void ResourceCache::ReleaseAllResources(bool force)
{
    bool released;
    do
    {
        released = false;

        for (auto i = resourceGroups_.begin();
             i != resourceGroups_.end(); ++i)
        {
            for (auto j = i->second.resources_.begin();
                 j != i->second.resources_.end();)
            {
                auto current = j++;
                // If other references exist, do not release, unless forced
                if ((current->second.use_count() == 1 && current->second.unique()) || force)
                {
                    i->second.resources_.erase(current);
                    released = true;
                }
            }
            if (released)
                UpdateResourceGroup(i->first);
        }

    } while (released && !force);
}

bool ResourceCache::ReloadResource(const String& resourceName)
{
    if (Resource* resource = FindResource(String::EMPTY, resourceName).get())
        return ReloadResource(resource);
    return false;
}

bool ResourceCache::ReloadResource(Resource* resource)
{
    if (!resource)
        return false;

    resource->onReloadStarted();

    bool success = false;
    AbstractFilePtr file = GetFile(resource->GetName());
    if (file)
        success = resource->Load(*(file.get()));

    if (success)
    {
        resource->ResetUseTimer();
        UpdateResourceGroup(resource->GetType());
        resource->onReloadFinished();
        return true;
    }

    // If reloading failed, do not remove the resource from cache, to allow for a new live edit to
    // attempt loading again
    resource->onReloadFailed();
    return false;
}

void ResourceCache::ReloadResourceWithDependencies(const String& fileName)
{
    String fileNameHash(fileName);
    // If the filename is a resource we keep track of, reload it
    const std::shared_ptr<Resource>& resource = FindResource(fileNameHash);
    if (resource)
    {
        SE_LOG_DEBUG("Reloading changed resource " + fileName);
        ReloadResource(resource.get());
    }
    // Always perform dependency resource check for resource loaded from XML file as it could be used in inheritance
    if (!resource || GetExtension(resource->GetName()) == ".xml")
    {
        // Check if this is a dependency resource, reload dependents
        auto j = dependentResources_.find(fileNameHash);
        if (j != dependentResources_.end())
        {
            // Reloading a resource may modify the dependency tracking structure. Therefore collect the
            // resources we need to reload first
            std::vector<std::shared_ptr<Resource>> dependents;
            dependents.reserve(j->second.size());

            for (auto k = j->second.begin(); k != j->second.end(); ++k)
            {
                const std::shared_ptr<Resource>& dependent = FindResource(*k);
                if (dependent)
                    dependents.push_back(dependent);
            }

            for (unsigned k = 0; k < dependents.size(); ++k)
            {
                SE_LOG_DEBUG("Reloading resource {} depending on {}", dependents[k]->GetName(), fileName);
                ReloadResource(dependents[k].get());
            }
        }
    }
}

void ResourceCache::SetMemoryBudget(String type, unsigned long long budget)
{
    resourceGroups_[type].memoryBudget_ = budget;
}

void ResourceCache::AddResourceRouter(std::shared_ptr<ResourceRouter> router, bool addAsFirst)
{
    // Check for duplicate
    for (unsigned i = 0; i < resourceRouters_.size(); ++i)
    {
        if (resourceRouters_[i] == router)
            return;
    }

    if (addAsFirst)
        resourceRouters_.insert(resourceRouters_.begin(), router);
    else
        resourceRouters_.push_back(router);
}

void ResourceCache::RemoveResourceRouter(std::shared_ptr<ResourceRouter> router)
{
    for (unsigned i = 0; i < resourceRouters_.size(); ++i)
    {
        if (resourceRouters_[i] == router)
        {
            resourceRouters_.erase(resourceRouters_.begin() + i);
            return;
        }
    }
}

AbstractFilePtr ResourceCache::GetFile(const String& name, bool sendEventOnFailure)
{
     const auto* vfs = VirtualFileSystem::Get();

    const FileIdentifier resolvedName = GetResolvedIdentifier(FileIdentifier::FromUri(name));
    auto file = vfs->OpenFile(resolvedName, FILE_READ);

    if (!file && sendEventOnFailure)
    {
        if (!resourceRouters_.empty() && !resolvedName)
            SE_LOG_ERROR("Resource request '{}' was blocked", name);
        else
            SE_LOG_ERROR("Could not find resource '{}'", resolvedName.ToUri());

        if (Thread::IsMainThread())
        {
            // E_RESOURCENOTFOUND
            onResourceNotFound(resolvedName ? resolvedName.ToUri() : name);
        }
    }

    return file;
}

Resource* ResourceCache::GetExistingResource(String type, const String& name)
{
    String sanitatedName = SanitateResourceName(name);

    if (!Thread::IsMainThread())
    {
        SE_LOG_ERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
        return nullptr;
    }

    // If empty name, return null pointer immediately
    if (sanitatedName.empty())
        return nullptr;

    String nameHash(sanitatedName);

    const std::shared_ptr<Resource>& existing = type != String::EMPTY ? FindResource(type, nameHash) : FindResource(nameHash);
    return existing.get();
}

std::shared_ptr<Resource> ResourceCache::GetResource(String type, const String& name, bool sendEventOnFailure)
{
    String sanitatedName = SanitateResourceName(name);

    if (!Thread::IsMainThread())
    {
        SE_LOG_ERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
        return nullptr;
    }

    // If empty name, return null pointer immediately
    if (sanitatedName.empty())
        return nullptr;

    String nameHash(sanitatedName);

#ifdef SE_THREADING
    // Check if the resource is being background loaded but is now needed immediately
    backgroundLoader_->WaitForResource(type, nameHash);
#endif

    const std::shared_ptr<Resource>& existing = FindResource(type, nameHash);
    if (existing)
        return existing;

    std::shared_ptr<Resource> resource;
    // Make sure the pointer is non-null and is a Resource subclass
    resource = ResourceCache::CreateResource(type);
//    resource = std::dynamic_pointer_cast<Resource>(context_->CreateObject(type));
    if (!resource)
    {
        SE_LOG_ERROR("Could not load unknown resource type {}", type);

        if (sendEventOnFailure)
        {
            // E_UNKNOWNRESOURCETYPE
            onUnknownResourceType(type);
        }

        return nullptr;
    }

    // Attempt to load the resource
    const AbstractFilePtr file = GetFile(sanitatedName, sendEventOnFailure);
    if (!file)
        return nullptr;   // Error is already logged

    SE_LOG_DEBUG("Loading resource " + sanitatedName);
    resource->SetName(sanitatedName);
    resource->SetAbsoluteFileName(file->GetAbsoluteName());

    if (!resource->Load(*(file.get())))
    {
        // Error should already been logged by corresponding resource descendant class
        if (sendEventOnFailure)
        {
            onLoadFailed(sanitatedName);
        }

        if (!returnFailedResources_)
            return nullptr;
    }

    // Store to cache
    resource->ResetUseTimer();
    resourceGroups_[type].resources_[nameHash] = resource;
    UpdateResourceGroup(type);

    return resource;
}

bool ResourceCache::BackgroundLoadResource(String type, const String& name, bool sendEventOnFailure, Resource* caller)
{
#ifdef SE_THREADING
    // If empty name, fail immediately
    String sanitatedName = SanitateResourceName(name);
    if (sanitatedName.empty())
        return false;

    // First check if already exists as a loaded resource
    String nameHash(sanitatedName);
    if (FindResource(type, nameHash) != noResource)
        return false;

    return backgroundLoader_->QueueResource(type, sanitatedName, sendEventOnFailure, caller);
#else
    // When threading not supported, fall back to synchronous loading
    return GetResource(type, name, sendEventOnFailure).get();
#endif
}

std::shared_ptr<Resource> ResourceCache::GetTempResource(String type, const String& name, bool sendEventOnFailure)
{
    String sanitatedName = SanitateResourceName(name);

    // If empty name, return null pointer immediately
    if (sanitatedName.empty())
        return std::shared_ptr<Resource>();

    std::shared_ptr<Resource> resource;
    // Make sure the pointer is non-null and is a Resource subclass
    resource = ResourceCache::CreateResource(type);
//    resource = std::dynamic_pointer_cast<Resource>(context_->CreateObject(type));
    if (!resource)
    {
        SE_LOG_ERROR("Could not load unknown resource type {}", type);

        if (sendEventOnFailure)
        {
            onUnknownResourceType(type);
        }

        return std::shared_ptr<Resource>();
    }

    // Attempt to load the resource
    AbstractFilePtr file = GetFile(sanitatedName, sendEventOnFailure);
    if (!file)
        return std::shared_ptr<Resource>();  // Error is already logged

    SE_LOG_DEBUG("Loading temporary resource " + sanitatedName);
    resource->SetName(file->GetName());
    resource->SetAbsoluteFileName(file->GetAbsoluteName());

    if (!resource->Load(*(file.get())))
    {
        // Error should already been logged by corresponding resource descendant class
        if (sendEventOnFailure)
        {
            // using namespace LoadFailed;

            // VariantMap& eventData = GetEventDataMap();
            // eventData[P_RESOURCENAME] = sanitatedName;
            // SendEvent(E_LOADFAILED, eventData);
            onLoadFailed(sanitatedName);
        }

        return std::shared_ptr<Resource>();
    }

    return resource;
}

unsigned ResourceCache::GetNumBackgroundLoadResources() const
{
#ifdef SE_THREADING
    return backgroundLoader_->GetNumQueuedResources();
#else
    return 0;
#endif
}

void ResourceCache::GetResources(std::vector<Resource*>& result, String type) const
{
    result.clear();
    auto i = resourceGroups_.find(type);
    if (i != resourceGroups_.end())
    {
        for (auto j = i->second.resources_.begin();
             j != i->second.resources_.end(); ++j)
            result.push_back(j->second.get());
    }
}

bool ResourceCache::Exists(const String& name) const
{
    const FileIdentifier resolvedName = GetResolvedIdentifier(FileIdentifier::FromUri(name));
    if (!resolvedName)
        return false;

    const auto vfs = VirtualFileSystem::Get();
    return vfs->Exists(resolvedName);
}

unsigned long long ResourceCache::GetMemoryBudget(String type) const
{
    auto i = resourceGroups_.find(type);
    return i != resourceGroups_.end() ? i->second.memoryBudget_ : 0;
}

unsigned long long ResourceCache::GetMemoryUse(String type) const
{
    auto i = resourceGroups_.find(type);
    return i != resourceGroups_.end() ? i->second.memoryUse_ : 0;
}

unsigned long long ResourceCache::GetTotalMemoryUse() const
{
    unsigned long long total = 0;
    for (auto i = resourceGroups_.begin(); i != resourceGroups_.end(); ++i)
        total += i->second.memoryUse_;
    return total;
}

String ResourceCache::GetResourceFileName(const String& name) const
{
    const auto vfs = VirtualFileSystem::Get();
    return vfs->GetAbsoluteNameFromIdentifier(FileIdentifier::FromUri(name));
}

ResourceRouter* ResourceCache::GetResourceRouter(unsigned index) const
{
    return index < resourceRouters_.size() ? resourceRouters_[index].get() : nullptr;
}

String ResourceCache::SanitateResourceName(const String& name) const
{
    return GetCanonicalIdentifier(FileIdentifier::FromUri(name)).ToUri();
}

void ResourceCache::StoreResourceDependency(Resource* resource, const String& dependency)
{
    if (!resource)
        return;

    MutexLock lock(resourceMutex_);

    String nameHash(resource->GetName());
    auto& dependents = dependentResources_[dependency];
    dependents.insert(nameHash);
}

void ResourceCache::ResetDependencies(Resource* resource)
{
    if (!resource)
        return;

    MutexLock lock(resourceMutex_);

    String nameHash(resource->GetName());

    for (auto i = dependentResources_.begin(); i != dependentResources_.end();)
    {
        auto& dependents = i->second;
        dependents.erase(nameHash);
        if (dependents.empty())
            i = dependentResources_.erase(i);
        else
            ++i;
    }
}

String ResourceCache::PrintMemoryUsage() const
{
    String output = "Resource Type                 Cnt       Avg       Max    Budget     Total\n\n";

    unsigned totalResourceCt = 0;
    unsigned long long totalLargest = 0;
    unsigned long long totalAverage = 0;
    unsigned long long totalUse = GetTotalMemoryUse();

    for (auto cit = resourceGroups_.begin(); cit != resourceGroups_.end(); ++cit)
    {
        const unsigned resourceCt = cit->second.resources_.size();
        unsigned long long average = 0;
        if (resourceCt > 0)
            average = cit->second.memoryUse_ / resourceCt;
        else
            average = 0;
        unsigned long long largest = 0;
        for (auto resIt = cit->second.resources_.begin(); resIt != cit->second.resources_.end(); ++resIt)
        {
            if (resIt->second->GetMemoryUse() > largest)
                largest = resIt->second->GetMemoryUse();
            if (largest > totalLargest)
                totalLargest = largest;
        }

        totalResourceCt += resourceCt;

        const String countString = format("{}", cit->second.resources_.size());
        const String memUseString = StringMemory(average);
        const String memMaxString = StringMemory(largest);
        const String memBudgetString = StringMemory(cit->second.memoryBudget_);
        const String memTotalString = StringMemory(cit->second.memoryUse_);
        const String resTypeName = cit->first;


        output += cformat("%-28s %4s %9s %9s %9s %9s\n", 
            resTypeName.c_str(), countString.c_str(), memUseString.c_str(), 
            memMaxString.c_str(), memBudgetString.c_str(), memTotalString.c_str());
    }

    if (totalResourceCt > 0)
        totalAverage = totalUse / totalResourceCt;

    const String countString = format("{}", totalResourceCt);
    const String memUseString = StringMemory(totalAverage);
    const String memMaxString = StringMemory(totalLargest);
    const String memTotalString = StringMemory(totalUse);

    output += cformat("%-28s %4s %9s %9s %9s %9s\n", 
        "All", countString.c_str(), memUseString.c_str(), memMaxString.c_str(), 
        "-", memTotalString.c_str());
    return output;
}

const std::shared_ptr<Resource>& ResourceCache::FindResource(String type, String nameHash)
{
    MutexLock lock(resourceMutex_);

    auto i = resourceGroups_.find(type);
    if (i == resourceGroups_.end())
        return noResource;
    auto j = i->second.resources_.find(nameHash);
    if (j == i->second.resources_.end())
        return noResource;

    return j->second;
}

const std::shared_ptr<Resource>& ResourceCache::FindResource(String nameHash)
{
    MutexLock lock(resourceMutex_);

    for (auto i = resourceGroups_.begin(); i != resourceGroups_.end(); ++i)
    {
        auto j = i->second.resources_.find(nameHash);
        if (j != i->second.resources_.end())
            return j->second;
    }

    return noResource;
}

void ResourceCache::ReleasePackageResources(PackageFile* package, bool force)
{
    std::unordered_set<String> affectedGroups;

    const auto& entries = package->GetEntries();
    for (auto i = entries.begin(); i != entries.end(); ++i)
    {
        String nameHash(i->first);

        // We do not know the actual resource type, so search all type containers
        for (auto j = resourceGroups_.begin(); j != resourceGroups_.end(); ++j)
        {
            auto k = j->second.resources_.find(nameHash);
            if (k != j->second.resources_.end())
            {
                // If other references exist, do not release, unless forced
                if ((k->second.use_count() == 1 && k->second.unique()) || force)
                {
                    j->second.resources_.erase(k);
                    affectedGroups.insert(j->first);
                }
                break;
            }
        }
    }

    for (auto i = affectedGroups.begin(); i != affectedGroups.end(); ++i)
        UpdateResourceGroup(*i);
}

void ResourceCache::UpdateResourceGroup(String type)
{
    auto i = resourceGroups_.find(type);
    if (i == resourceGroups_.end())
        return;

    for (;;)
    {
        unsigned totalSize = 0;
        unsigned oldestTimer = 0;
        auto oldestResource = i->second.resources_.end();

        for (auto j = i->second.resources_.begin();
             j != i->second.resources_.end(); ++j)
        {
            totalSize += j->second->GetMemoryUse();
            unsigned useTimer = j->second->GetUseTimer();
            if (useTimer > oldestTimer)
            {
                oldestTimer = useTimer;
                oldestResource = j;
            }
        }

        i->second.memoryUse_ = totalSize;

        // If memory budget defined and is exceeded, remove the oldest resource and loop again
        // (resources in use always return a zero timer and can not be removed)
        if (i->second.memoryBudget_ && i->second.memoryUse_ > i->second.memoryBudget_ &&
            oldestResource != i->second.resources_.end())
        {
            SE_LOG_DEBUG("Resource group {} over memory budget, releasing resource {}",
                     oldestResource->second->GetType(), oldestResource->second->GetName());
            i->second.resources_.erase(oldestResource);
        }
        else
            break;
    }
}

void ResourceCache::HandleBeginFrame()
{
   
    // Check for background loaded resources that can be finished
#ifdef SE_THREADING
    {
        SE_PROFILE("FinishBackgroundResources");
        backgroundLoader_->FinishResources(finishBackgroundResourcesMs_);
    }
#endif
}

void ResourceCache::HandleFileChanged(const FileChangeInfo& fileInfo)
{
    // if (fileInfo.kind == FileChangeKind::FILECHANGE_MODIFIED)
    //     return;

    auto it = std::find(ignoreResourceAutoReload_.begin(), ignoreResourceAutoReload_.end(), fileInfo.resourceName);
    if (it != ignoreResourceAutoReload_.end())
    {
        ignoreResourceAutoReload_.erase(it);
        return;
    }

    ReloadResourceWithDependencies(fileInfo.resourceName);
}

// void RegisterResourceLibrary(Context* context)
// {
//     BinaryFile::RegisterObject(context);
//     Image::RegisterObject(context);
//     ImageCube::RegisterObject(context);
//     JSONFile::RegisterObject(context);
//     PListFile::RegisterObject(context);
//     XMLFile::RegisterObject(context);
//     // Graph::RegisterObject(context);
//     // GraphNode::RegisterObject(context);
//     SerializableResource::RegisterObject(context);
// }


void ResourceCache::Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    auto vfs = VirtualFileSystem::Get();
    vfs->Scan(result, FileIdentifier::FromUri(pathName), filter, flags);

    // Scan manual resources.
    if (!flags.Test(SCAN_FILES))
        return;

    const bool recursive = flags.Test(SCAN_RECURSIVE);
    String filterExtension = GetExtensionFromFilter(filter);
    for (const auto& [_, group] : resourceGroups_)
    {
        for (const auto& [_, resource] : group.resources_)
        {
            if (!MatchFileName(resource->GetName(), pathName, filterExtension, recursive))
                continue;

            const FileIdentifier resourceName = FileIdentifier::FromUri(resource->GetName());
            if (!vfs->Exists(resourceName))
                result.emplace_back(TrimPathPrefix(resource->GetName(), pathName));
        }
    }
}

String ResourceCache::PrintResources(const String& typeName) const
{

    String typeNameHash(typeName);

    String output = "Resource Type         Refs   WeakRefs  Name\n\n";

    for (auto cit : resourceGroups_)
    {
        for (auto resIt : cit.second.resources_)
        {
            auto resource = resIt.second;

            // filter
            if (typeName.length() && resource->GetTypeHash() != typeNameHash)
                continue;

            output += cformat("%s     %i     %i     %s\n",
                                  resource->GetType().c_str(), resource.use_count(),
                                  resource.unique(), resource->GetName().c_str());
        }

    }

    return output;
}



void ResourceCache::IgnoreResourceReload(const String& name)
{
    ignoreResourceAutoReload_.emplace_back(name);
}

void ResourceCache::IgnoreResourceReload(const Resource* resource)
{
    IgnoreResourceReload(resource->GetName());
}


void ResourceCache::RouteResourceName(FileIdentifier& name) const
{
    auto vfs = VirtualFileSystem::Get();
    name = vfs->GetCanonicalIdentifier(name);

    thread_local bool reentrancyGuard = false;
    if (reentrancyGuard)
        return;

    reentrancyGuard = true;
    for (auto router : resourceRouters_)
        router->Route(name);
    reentrancyGuard = false;
}

void ResourceCache::Clear()
{
    resourceGroups_.clear();
    dependentResources_.clear();
}

FileIdentifier ResourceCache::GetCanonicalIdentifier(const FileIdentifier& name) const
{
    auto* vfs = VirtualFileSystem::Get();
    return vfs->GetCanonicalIdentifier(name);
}

FileIdentifier ResourceCache::GetResolvedIdentifier(const FileIdentifier& name) const
{
    FileIdentifier result = name;
    RouteResourceName(result);
    return result;
}

// void ResourceCache::HandleReflectionRemoved(ObjectReflection* reflection)
// {
//     ReleaseResources(reflection->GetTypeNameHash(), true);
// }

ResourceCache& ResourceCache::Get()
{
    static ResourceCache* ptr_;
    if (!ptr_) {
        SE_LOG_INFO("ResourceCache initialized.");
        ptr_ = new ResourceCache();
    }
    return *ptr_;
}

}
