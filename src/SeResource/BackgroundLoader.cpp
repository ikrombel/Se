#ifdef SE_THREADING


//#include <GFrost/Core/Context.h>
#include <Se/Profiler.hpp>
#include <Se/Console.hpp>
#include "BackgroundLoader.h"
#include "ResourceCache.h"
//#include <SeResource/ResourceEvents.h>

namespace Se
{
// template<class T>
// T::iterator FindPairStringHash(T map, const std::pair<StringHash, StringHash>& p) {
//     auto j = std::find(map.begin(), map.end(), [p](const std::pair<StringHash, StringHash>& param){
//             return p.first == param.first && p.second == param.second;
//         });
//     return j;
//  }

BackgroundLoader::BackgroundLoader(ResourceCache* owner) :
    owner_(owner)
{
}

BackgroundLoader::~BackgroundLoader()
{
    MutexLock lock(backgroundLoadMutex_);

    backgroundLoadQueue_.clear();
}

void BackgroundLoader::ThreadFunction()
{
    SE_PROFILE_THREAD("BackgroundLoader Thread");

    while (shouldRun_)
    {
        backgroundLoadMutex_.Acquire();

        // Search for a queued resource that has not been loaded yet
        auto i = backgroundLoadQueue_.begin();
        while (i != backgroundLoadQueue_.end())
        {
            if (i->second.resource_->GetAsyncLoadState() == ASYNC_QUEUED)
                break;
            else
                ++i;
        }

        if (i == backgroundLoadQueue_.end())
        {
            // No resources to load found
            backgroundLoadMutex_.Release();
            Time::Sleep(5);
        }
        else
        {
            BackgroundLoadItem& item = i->second;
            Resource* resource = item.resource_.get();
            // We can be sure that the item is not removed from the queue as long as it is in the
            // "queued" or "loading" state
            backgroundLoadMutex_.Release();

            bool success = false;
            AbstractFilePtr file = owner_->GetFile(resource->GetName(), item.sendEventOnFailure_);
            if (file)
            {
                resource->SetAsyncLoadState(ASYNC_LOADING);
                success = resource->BeginLoad(*file);
            }

            // Process dependencies now
            // Need to lock the queue again when manipulating other entries
            auto key = std::make_pair(resource->GetTypeHash(), resource->GetNameHash());
            backgroundLoadMutex_.Acquire();
            if (item.dependents_.size())
            {
                for (auto depend = item.dependents_.begin(); depend != item.dependents_.end(); ++depend)
                {
                    auto j = backgroundLoadQueue_.find(*depend);
                    // auto j = std::find(backgroundLoadQueue_.begin(), backgroundLoadQueue_.end(), [depend](const std::pair<StringHash, StringHash>& param){
                    //     return depend->first == param.first && depend->second == param.second;
                    // });
                    //auto j = FindPairStringHash(backgroundLoadQueue_, *depend);
                    if (j != backgroundLoadQueue_.end()) {
                        // auto jk = std::find(j->second.dependencies_.begin(), j->second.dependencies_.end(), [key](const std::pair<StringHash, StringHash>& param){
                        //     return key.first == param.first && key.second == param.second;
                        // });
                        //auto jk = FindPairStringHash(j->second.dependencies_, key);
                        auto jk = j->second.dependencies_.find(key);
                        j->second.dependencies_.erase(jk); //erase
                    }
                }

                item.dependents_.clear();
            }

            resource->SetAsyncLoadState(success ? ASYNC_SUCCESS : ASYNC_FAIL);
            backgroundLoadMutex_.Release();
        }
    }
}

bool BackgroundLoader::QueueResource(StringHash type, const String& name, bool sendEventOnFailure, Resource* caller)
{
    StringHash nameHash(name);
    auto key = std::make_pair(type, nameHash);

    MutexLock lock(backgroundLoadMutex_);

    // Check if already exists in the queue
//    auto it = FindPairStringHash(backgroundLoadQueue_, key);
    auto it = backgroundLoadQueue_.find(key);
    if (it != backgroundLoadQueue_.end())
        return false;

    // auto item = std::find(backgroundLoadQueue_.begin(), backgroundLoadQueue_.end(), [key](const std::pair<StringHash, StringHash>& param) {
    //         return key.first == param.first && key.second == param.second;
    // });
    BackgroundLoadItem& item = backgroundLoadQueue_[key];
    item.sendEventOnFailure_ = sendEventOnFailure;

    // Make sure the pointer is non-null and is a Resource subclass
    item.resource_ = ResourceCache::CreateResource(type);
    // DynamicCast<Resource>(owner_->GetContext()->CreateObject(type));
    if (!item.resource_)
    {
        SE_LOG_ERROR("Could not load unknown resource type {}", type.ToDebugString());

        if (sendEventOnFailure && Thread::IsMainThread())
        {
            // using namespace UnknownResourceType;

            // VariantMap& eventData = owner_->GetEventDataMap();
            // eventData[P_RESOURCETYPE] = type;
            // owner_->SendEvent(E_UNKNOWNRESOURCETYPE, eventData);
            owner_->onUnknownResourceType(type);
        }
        auto itErase = backgroundLoadQueue_.find(key);
        //auto itErase = FindPairStringHash(backgroundLoadQueue_, key);
        backgroundLoadQueue_.erase(itErase);
        return false;
    }

    SE_LOG_DEBUG("Background loading resource " + name);

    item.resource_->SetName(name);
    item.resource_->SetAsyncLoadState(ASYNC_QUEUED);

    // If this is a resource calling for the background load of more resources, mark the dependency as necessary
    if (caller)
    {
        auto callerKey = std::make_pair(caller->GetTypeHash(), caller->GetNameHash());
//        auto j = FindPairStringHash(backgroundLoadQueue_, callerKey);
        auto j = backgroundLoadQueue_.find(callerKey);
        if (j != backgroundLoadQueue_.end())
        {
            BackgroundLoadItem& callerItem = j->second;
            item.dependents_.insert(callerKey);
            callerItem.dependencies_.insert(key);
        }
        else
            SE_LOG_WARNING("Resource {} requested for a background loaded resource but was not in the background load queue",
                    caller->GetName());
    }

    // Start the background loader thread now
    if (!IsStarted())
        Run();

    return true;
}

void BackgroundLoader::WaitForResource(StringHash type, StringHash nameHash)
{
    backgroundLoadMutex_.Acquire();

    // Check if the resource in question is being background loaded
    auto key = std::make_pair(type, nameHash);
    
    auto i = backgroundLoadQueue_.find(key);
//    auto i = FindPairStringHash(backgroundLoadQueue_, key);
    if (i != backgroundLoadQueue_.end())
    {
        backgroundLoadMutex_.Release();

        {
            Resource* resource = i->second.resource_.get();
            HiresTimer waitTimer;
            bool didWait = false;

            for (;;)
            {
                unsigned numDeps = i->second.dependencies_.size();
                AsyncLoadState state = resource->GetAsyncLoadState();
                if (numDeps > 0 || state == ASYNC_QUEUED || state == ASYNC_LOADING)
                {
                    didWait = true;
                    Time::Sleep(1);
                }
                else
                    break;
            }

            if (didWait)
                SE_LOG_DEBUG("Waited {} ms for background loaded resource {}",
                         waitTimer.GetUSec(false) / 1000, resource->GetName());
        }

        // This may take a long time and may potentially wait on other resources, so it is important we do not hold the mutex during this
        FinishBackgroundLoading(i->second);

        backgroundLoadMutex_.Acquire();
        backgroundLoadQueue_.erase(i);
        backgroundLoadMutex_.Release();
    }
    else
        backgroundLoadMutex_.Release();
}

void BackgroundLoader::FinishResources(int maxMs)
{
    if (IsStarted())
    {
        HiresTimer timer;

        backgroundLoadMutex_.Acquire();

        for (auto i = backgroundLoadQueue_.begin(); i != backgroundLoadQueue_.end();)
        {
            const auto key = i->first;
            Resource* resource = i->second.resource_.get();
            unsigned numDeps = i->second.dependencies_.size();
            AsyncLoadState state = resource->GetAsyncLoadState();
            if (numDeps > 0 || state == ASYNC_QUEUED || state == ASYNC_LOADING)
                ++i;
            else
            {
                // Finishing a resource may need it to wait for other resources to load, in which case we can not
                // hold on to the mutex
                backgroundLoadMutex_.Release();
                FinishBackgroundLoading(i->second);
                backgroundLoadMutex_.Acquire();
                backgroundLoadQueue_.erase(key);
            }

            // Break when the time limit passed so that we keep sufficient FPS
            if (timer.GetUSec(false) >= maxMs * 1000LL)
                break;
        }

        backgroundLoadMutex_.Release();
    }
}

unsigned BackgroundLoader::GetNumQueuedResources() const
{
    MutexLock lock(backgroundLoadMutex_);
    return backgroundLoadQueue_.size();
}

void BackgroundLoader::FinishBackgroundLoading(BackgroundLoadItem& item)
{
    auto resource = item.resource_;

    bool success = resource->GetAsyncLoadState() == ASYNC_SUCCESS;
    // If BeginLoad() phase was successful, call EndLoad() and get the final success/failure result
    if (success)
    {
        SE_PROFILE("FinishBackgroundLoading");
//        SE_PROFILE_ZONENAME(resource->GetTypeName().c_str()(), resource->GetTypeName().length());
        SE_LOG_DEBUG("Finishing background loaded resource " + resource->GetName());
        success = resource->EndLoad();
    }
    resource->SetAsyncLoadState(ASYNC_DONE);

    if (!success && item.sendEventOnFailure_)
    {
        //E_LOADFAILED
        owner_->onLoadFailed(resource->GetName());
    }

    // Store to the cache just before sending the event; use same mechanism as for manual resources
    if (success || owner_->GetReturnFailedResources())
        owner_->AddManualResource(resource);

    //E_RESOURCEBACKGROUNDLOADED
    owner_->onResourceBackgroundLoaded(resource->GetName(), resource, success);
}

}

#endif
