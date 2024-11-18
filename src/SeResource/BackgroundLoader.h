#pragma once

#include <Se/Algorithms.hpp>
#include <Se/Mutex.hpp>
#include <Se/Thread.h>
#include <Se/StringHash.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>

// Register span with std::hash
namespace std {
template <>
struct hash<std::pair<Se::StringHash, Se::StringHash>> {
    size_t operator()(const std::pair<Se::StringHash, Se::StringHash>& s) const {
        std::size_t hash{0};
        Se::hash_combine(hash, s.first.ToHash());
        Se::hash_combine(hash, s.second.ToHash());
        return  hash;
    }
};
} // namespace std

namespace Se
{

class Resource;
class ResourceCache;

/// Queue item for background loading of a resource.
struct BackgroundLoadItem
{
    /// Resource.
    std::shared_ptr<Resource> resource_;
    /// Resources depended on for loading.
    std::unordered_set<std::pair<StringHash, StringHash> > dependencies_;
    /// Resources that depend on this resource's loading.
    std::unordered_set<std::pair<StringHash, StringHash> > dependents_;
    /// Whether to send failure event.
    bool sendEventOnFailure_;
};

/// Background loader of resources. Owned by the ResourceCache.
class BackgroundLoader : public Thread
{
public:
    /// Construct.
    explicit BackgroundLoader(ResourceCache* owner);

    /// Destruct. Forcibly clear the load queue.
    virtual ~BackgroundLoader();

    /// Resource background loading loop.
    void ThreadFunction() override;

    /// Queue loading of a resource. The name must be sanitated to ensure consistent format. Return true if queued (not a duplicate and resource was a known type).
    bool QueueResource(StringHash type, const String& name, bool sendEventOnFailure, Resource* caller);
    /// Wait and finish possible loading of a resource when being requested from the cache.
    void WaitForResource(StringHash type, StringHash nameHash);
    /// Process resources that are ready to finish.
    void FinishResources(int maxMs);

    /// Return amount of resources in the load queue.
    unsigned GetNumQueuedResources() const;

private:
    /// Finish one background loaded resource.
    void FinishBackgroundLoading(BackgroundLoadItem& item);

    /// Resource cache.
    ResourceCache* owner_;
    /// Mutex for thread-safe access to the background load queue.
    mutable Mutex backgroundLoadMutex_;
    /// Resources that are queued for background loading.
    std::unordered_map<std::pair<StringHash, StringHash>, BackgroundLoadItem> backgroundLoadQueue_;
};

}
