#pragma once

#include <Se/Mutex.hpp>
#include <Se/IO/File.h>
#include <Se/String.hpp>
#include <SeVFS/FileIdentifier.h>
#include <SeVFS/FileWatcher.h>
#include <Se/IO/ScanFlags.hpp>
#include <SeResource/Resource.h>

#include <unordered_set>
#include <unordered_map>

namespace Se
{

class BackgroundLoader;
class FileWatcher;
class PackageFile;

/// Sets to priority so that a package or file is pushed to the end of the vector.
static const unsigned PRIORITY_LAST = 0xffffffff;

/// Container of resources with specific type.
struct ResourceGroup
{
    /// Construct with defaults.
    ResourceGroup() :
        memoryBudget_(0),
        memoryUse_(0)
    {
    }

    /// Memory budget.
    unsigned long long memoryBudget_;
    /// Current memory use.
    unsigned long long memoryUse_;
    /// Resources.
    std::unordered_map<String, std::shared_ptr<Resource>> resources_;
};

/// Optional resource request processor. Can deny requests, re-route resource file names, or perform other processing per request.
class ResourceRouter
{
public:
    /// Construct.
    explicit ResourceRouter()
    {
    }

    /// Process the resource request and optionally modify the resource name string. Empty name string means the resource is not found or not allowed.
    virtual void Route(FileIdentifier& name) = 0;
};

/// %Resource cache subsystem. Loads resources on demand and stores them for later access.
class ResourceCache// : public Object
{
//    GFROST_OBJECT(ResourceCache, Object);

public:
    /// E_LOADFAILED
    Signal<String/*ResourceName*/> onLoadFailed;
    /// E_RESOURCENOTFOUND
    Signal<String/*ResourceName*/> onResourceNotFound;
    /// E_UNKNOWNRESOURCETYPE
    Signal<String/*ResourceType*/> onUnknownResourceType;
    /// E_RESOURCEBACKGROUNDLOADED
    Signal<
        String/*ResourceName*/,
        std::shared_ptr<Resource> /*resource*/,
        bool /*success*/> onResourceBackgroundLoaded;

    /// Construct.
    explicit ResourceCache();
    /// Destruct. Free all resources.
    virtual ~ResourceCache();

    /// Add a manually created resource. Must be uniquely named within its type.
    bool AddManualResource(std::shared_ptr<Resource> resource);
    /// Release a resource by name.
    void ReleaseResource(String type, const String& name, bool force = false);
    /// Release a resource by name.
    void ReleaseResource(const String& resourceName, bool force = false);
    /// Release all resources of a specific type.
    void ReleaseResources(String type, bool force = false);
    /// Release resources of a specific type and partial name.
    void ReleaseResources(String type, const String& partialName, bool force = false);
    /// Release resources of all types by partial name.
    void ReleaseResources(const String& partialName, bool force = false);
    /// Release all resources. When called with the force flag false, releases all currently unused resources.
    void ReleaseAllResources(bool force = false);
    /// Reload a resource. Return true on success. The resource will not be removed from the cache in case of failure.
    bool ReloadResource(const String& resourceName);
    /// Reload a resource. Return true on success. The resource will not be removed from the cache in case of failure.
    bool ReloadResource(Resource* resource);
    /// Reload a resource based on filename. Causes also reload of dependent resources if necessary.
    void ReloadResourceWithDependencies(const String& fileName);
    /// Set memory budget for a specific resource type, default 0 is unlimited.
    void SetMemoryBudget(String type, unsigned long long budget);
    /// Enable or disable returning resources that failed to load. Default false. This may be useful in editing to not lose resource ref attributes.
    void SetReturnFailedResources(bool enable) { returnFailedResources_ = enable; }

    /// Define whether when getting resources should check package files or directories first. True for packages, false for directories.
    void SetSearchPackagesFirst(bool value) { searchPackagesFirst_ = value; }

    /// Set how many milliseconds maximum per frame to spend on finishing background loaded resources.
    void SetFinishBackgroundResourcesMs(int ms) { finishBackgroundResourcesMs_ = std::max(ms, 1); }

    /// Add a resource router object. By default there is none, so the routing process is skipped.
    void AddResourceRouter(std::shared_ptr<ResourceRouter> router, bool addAsFirst = false);
    /// Remove a resource router object.
    void RemoveResourceRouter(std::shared_ptr<ResourceRouter> router);

    /// Open and return a file from the resource load paths or from inside a package file. If not found, use a fallback search with absolute path. Return null if fails. Can be called from outside the main thread.
    AbstractFilePtr GetFile(const String& name, bool sendEventOnFailure = true);
    /// Return a resource by type and name. Load if not loaded yet. Return null if not found or if fails, unless SetReturnFailedResources(true) has been called. Can be called only from the main thread.
    std::shared_ptr<Resource> GetResource(String type, const String& name, bool sendEventOnFailure = true);
    /// Load a resource without storing it in the resource cache. Return null if not found or if fails. Can be called from outside the main thread if the resource itself is safe to load completely (it does not possess for example GPU data.)
    std::shared_ptr<Resource> GetTempResource(String type, const String& name, bool sendEventOnFailure = true);
    /// Background load a resource. An event will be sent when complete. Return true if successfully stored to the load queue, false if eg. already exists. Can be called from outside the main thread.
    bool BackgroundLoadResource(String type, const String& name, bool sendEventOnFailure = true, Resource* caller = nullptr);
    /// Return number of pending background-loaded resources.
    unsigned GetNumBackgroundLoadResources() const;
    /// Return all loaded resources of a specific type.
    void GetResources(std::vector<Resource*>& result, String type) const;
    /// Return an already loaded resource of specific type & name, or null if not found. Will not load if does not exist.
    Resource* GetExistingResource(String type, const String& name);

    /// Return all loaded resources.
    const std::unordered_map<String, ResourceGroup>& GetAllResources() const { return resourceGroups_; }

    /// Template version of returning a resource by name.
    template <class T> T* GetResource(const String& name, bool sendEventOnFailure = true);
    /// Template version of returning an existing resource by name.
    template <class T> T* GetExistingResource(const String& name);
    /// Template version of loading a resource without storing it to the cache.
    template <class T> std::shared_ptr<T> GetTempResource(const String& name, bool sendEventOnFailure = true);
    /// Template version of releasing a resource by name.
    template <class T> void ReleaseResource(const String& name, bool force = false);
    /// Template version of queueing a resource background load.
    template <class T> bool BackgroundLoadResource(const String& name, bool sendEventOnFailure = true, Resource* caller = nullptr);
    /// Template version of returning loaded resources of a specific type.
    template <class T> void GetResources(std::vector<T*>& result) const;
    /// Return whether a file exists in the resource directories or package files. Does not check manually added in-memory resources.
    bool Exists(const String& name) const;
    /// Return memory budget for a resource type.
    unsigned long long GetMemoryBudget(String type) const;
    /// Return total memory use for a resource type.
    unsigned long long GetMemoryUse(String type) const;
    /// Return total memory use for all resources.
    unsigned long long GetTotalMemoryUse() const;
    /// Return full absolute file name of resource if possible, or empty if not found.
    String GetResourceFileName(const String& name) const;

    /// Return whether resources that failed to load are returned.
    bool GetReturnFailedResources() const { return returnFailedResources_; }

    /// Return whether when getting resources should check package files or directories first.
    bool GetSearchPackagesFirst() const { return searchPackagesFirst_; }

    /// Return how many milliseconds maximum to spend on finishing background loaded resources.
    int GetFinishBackgroundResourcesMs() const { return finishBackgroundResourcesMs_; }

    /// Return a resource router by index.
    ResourceRouter* GetResourceRouter(unsigned index) const;


    /// Remove unsupported constructs from the resource name to prevent ambiguity, and normalize absolute filename to resource path relative if possible.
    String SanitateResourceName(const String& name) const;
    /// Store a dependency for a resource. If a dependency file changes, the resource will be reloaded.
    void StoreResourceDependency(Resource* resource, const String& dependency);
    /// Reset dependencies for a resource.
    void ResetDependencies(Resource* resource);

    /// Returns a formatted string containing the memory actively used.
    String PrintMemoryUsage() const;

    /// Scan for specified files.
    void Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const;
    /// Returns a formatted string containing the currently loaded resources with optional type name filter.
    String PrintResources(const String& typeName = String::EMPTY) const;
    /// When resource auto-reloading is enabled ignore reloading resource once.
    void IgnoreResourceReload(const String& name);
    /// When resource auto-reloading is enabled ignore reloading resource once.
    void IgnoreResourceReload(const Resource* resource);
    /// Pass name through resource routers and return final resource name.
    void RouteResourceName(FileIdentifier& name) const;
    /// Clear all resources from resource cache.
    void Clear();

    /// Return canonical resource identifier without resource routing.
    FileIdentifier GetCanonicalIdentifier(const FileIdentifier& name) const;
    /// Return canonical resource identifier with resource routing applied.
    FileIdentifier GetResolvedIdentifier(const FileIdentifier& name) const;

    static ResourceCache& Get();

    template<class T>
    static void RegisterResource(const String& resourceName, std::function<std::shared_ptr<Resource>()> func = nullptr)
    {
        auto resIt = resourceFactory_.find(resourceName);

        if (resIt != resourceFactory_.end())
        {
            SE_LOG_WARNING("Resource {} already registered. Overriding previous state", resourceName);
        }

        std::function<std::shared_ptr<Resource>()> defaultConstructor = []()
        {
            return std::make_shared<T>();
        };
        //resourceNames_.insert(resourceName, resourceName);
        resourceFactory_[resourceName] = func ? func : defaultConstructor;
    }

   static std::shared_ptr<Resource> CreateResource(const String& resourceName)
    {   
        auto resIt = resourceFactory_.find(resourceName);

        if (resIt == resourceFactory_.end())
        {
            SE_LOG_ERROR("Resource {} is not registered by RegisterResource", resourceName);
            return nullptr;
        }

        return resourceFactory_[resourceName](); 
    }

    template<class T>
    static std::shared_ptr<T> CreateResourceCast(const String& resourceName)
    {
        return std::dynamic_pointer_cast<T>(CreateResource(resourceName)); 
    }

private:
    /// Find a resource.
    const std::shared_ptr<Resource>& FindResource(String type, String nameHash);
    /// Find a resource by name only. Searches all type groups.
    const std::shared_ptr<Resource>& FindResource(String nameHash);
    /// Release resources loaded from a package file.
    void ReleasePackageResources(PackageFile* package, bool force = false);
    /// Update a resource group. Recalculate memory use and release resources if over memory budget.
    void UpdateResourceGroup(String type);
    /// Handle begin frame event. The finalization of background loaded resources are processed here.
    void HandleBeginFrame();
    /// Handle file changed to reload resource.
    void HandleFileChanged(const FileChangeInfo& fileInfo);
    // /// Handle object reflection removed.
    // void HandleReflectionRemoved(ObjectReflection* reflection);


    /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
    mutable Mutex resourceMutex_;
    /// Resources by type.
    std::unordered_map<String, ResourceGroup> resourceGroups_;
    /// Dependent resources. Only used with automatic reload to eg. trigger reload of a cube texture when any of its faces change.
    std::unordered_map<String, std::unordered_set<String>> dependentResources_;
    /// Resource background loader.
    std::shared_ptr<BackgroundLoader> backgroundLoader_;
    /// Resource routers.
    std::vector<std::shared_ptr<ResourceRouter> > resourceRouters_;

    /// Return failed resources flag.
    bool returnFailedResources_;
    /// Search priority flag.
    bool searchPackagesFirst_;

    /// How many milliseconds maximum per frame to spend on finishing background loaded resources.
    int finishBackgroundResourcesMs_;
    /// List of resources that will not be auto-reloaded if reloading event triggers.
    std::vector<String> ignoreResourceAutoReload_;



    static std::unordered_map<String, std::function<std::shared_ptr<Resource>()>> resourceFactory_;
    //static std::unordered_map<String, String> resourceNames_;
};

template <class T> T* ResourceCache::GetExistingResource(const String& name)
{
    String type = T::GetTypeStatic();
    return dynamic_cast<T*>(GetExistingResource(type, name));
}

template <class T> T* ResourceCache::GetResource(const String& name, bool sendEventOnFailure)
{
    String type = T::GetTypeStatic();
    return dynamic_cast<T*>(GetResource(type, name, sendEventOnFailure));
}

template <class T> void ResourceCache::ReleaseResource(const String& name, bool force)
{
    String type = T::GetTypeStatic();
    ReleaseResource(type, name, force);
}

template <class T> std::shared_ptr<T> ResourceCache::GetTempResource(const String& name, bool sendEventOnFailure)
{
    String type = T::GetTypeStatic();
    return std::dynamic_pointer_cast<T>(GetTempResource(type, name, sendEventOnFailure));
}

template <class T> bool ResourceCache::BackgroundLoadResource(const String& name, bool sendEventOnFailure, Resource* caller)
{
    String type = T::GetTypeStatic();
    return BackgroundLoadResource(type, name, sendEventOnFailure, caller);
}

template <class T> void ResourceCache::GetResources(std::vector<T*>& result) const
{
    auto& resources = reinterpret_cast<std::vector<Resource*>&>(result);
    String type = T::GetTypeStatic();
    GetResources(resources, type);

    // Perform conversion of the returned pointers
    for (unsigned i = 0; i < result.size(); ++i)
    {
        Resource* resource = resources[i];
        result[i] = static_cast<T*>(resource);
    }
}

/// Register Resource library subsystems and objects.
void SE_API RegisterResourceLibrary();

template<typename T>
struct ResourceRegistrator {
  ResourceRegistrator(const char* typeName) {
    ResourceCache::RegisterResource<T>(typeName);
  }
};

#define REGISTER_RESOURCE(typeName) \
    namespace ResourceRegScope { \
        static ResourceRegistrator<typeName> reg_##typeName(#typeName); \
    }

}
