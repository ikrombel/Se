

#pragma once

//#include <SeMath/MathDefs.hpp>
#include <Se/MultiVector.hpp>
#include <Se/Mutex.hpp>
#include <Se/Signal.hpp>

#include <atomic>
#include <list>
#include <memory>
#include <functional>

namespace Se
{

/// Work item completed event.
// GFROST_EVENT(E_WORKITEMCOMPLETED, WorkItemCompleted)
// {
//     GFROST_PARAM(P_ITEM, Item);                        // WorkItem ptr
// }

class WorkerThread;

/// Vector-like collection that can be safely filled from different WorkQueue threads simultaneously.
template <class T>
class WorkQueueVector : public MultiVector<T>
{
public:
    /// Clear collection, considering number of threads in WorkQueue.
    void Clear();
    /// Insert new element. Thread-safe as long as called from WorkQueue threads (or main thread).
    auto Insert(const T& value);
    /// Emplace element. Thread-safe as long as called from WorkQueue threads (or main thread).
    template <class ... Args>
    T& Emplace(Args&& ... args);
};

/// Task function signature.
/// TODO: Get rid of parameter
using WorkFunction = std::function<void(unsigned threadIndex)>;


/// Work queue item.
struct WorkItem// : public RefCounted
{
    friend class WorkQueue;

public:
    /// Work function. Called with the work item and thread index (0 = main thread) as parameters.
    void (* workFunction_)(const WorkItem*, unsigned){};
    /// Data start pointer.
    void* start_{};
    /// Data end pointer.
    void* end_{};
    /// Auxiliary data pointer.
    void* aux_{};
    /// Priority. Higher value = will be completed first.
    unsigned priority_{};
    /// Whether to send event on completion.
    bool sendEvent_{};
    /// Completed flag.
    std::atomic<bool> completed_{};

private:
    bool pooled_{};
    /// Work function. Called without any parameters.
    WorkFunction workLambda_;
};

/// Work queue subsystem for multithreading.
class WorkQueue// : public Object
{
//    GFROST_OBJECT(WorkQueue, Object);

    friend class WorkerThread;

public:
    /// Work item completed event. E_WORKITEMCOMPLETED
    Signal<WorkItem*> onWorkItemCompleted;

    Signal<> onWorkCompleted;

    /// Construct.
    explicit WorkQueue();
    /// Destruct.
    virtual ~WorkQueue(); // override;

    /// Create worker threads. Can only be called once.
    void CreateThreads(unsigned numThreads);

    /// Invoke callback from main thread. May be called immediately.
    void CallFromMainThread(WorkFunction workFunction);

    /// Get pointer to an usable WorkItem from the item pool. Allocate one if no more free items.
    std::shared_ptr<WorkItem> GetFreeItem();
    /// Add a work item and resume worker threads.
    void AddWorkItem(const std::shared_ptr<WorkItem>& item);
    /// Add a work item and resume worker threads.
    std::shared_ptr<WorkItem> AddWorkItem(WorkFunction workFunction, unsigned priority = 0);
    /// }@

    /// Remove a work item before it has started executing. Return true if successfully removed.
    bool RemoveWorkItem(std::shared_ptr<WorkItem> item);
    /// Remove a number of work items before they have started executing. Return the number of items successfully removed.
    unsigned RemoveWorkItems(const std::vector<std::shared_ptr<WorkItem> >& items);
    /// Pause worker threads.
    void Pause();
    /// Resume worker threads.
    void Resume();
    /// Finish all queued work which has at least the specified priority. Main thread will also execute priority work. Pause worker threads if no more work remains.
    void Complete(unsigned priority);

    /// Set the pool telerance before it starts deleting pool items.
    void SetTolerance(int tolerance) { tolerance_ = tolerance; }

    /// Set how many milliseconds maximum per frame to spend on low-priority work, when there are no worker threads.
    void SetNonThreadedWorkMs(int ms) { maxNonThreadedWorkMs_ = std::max(ms, 1); }

    /// Return number of worker threads.
    std::size_t GetNumThreads() const { return threads_.size(); }

    /// Return number of incomplete tasks with at least the specified priority.
    unsigned GetNumIncomplete(unsigned priority) const;
    /// Return whether all work with at least the specified priority is finished.
    bool IsCompleted(unsigned priority) const;
    /// Return whether the queue is currently completing work in the main thread.
    bool IsCompleting() const { return completing_; }

    /// Return the pool tolerance.
    int GetTolerance() const { return tolerance_; }

    /// Return how many milliseconds maximum to spend on non-threaded low-priority work.
    int GetNonThreadedWorkMs() const { return maxNonThreadedWorkMs_; }

    /// Return current thread index.
    static unsigned GetThreadIndex();
    /// Return number of threads used by WorkQueue, including main thread. Current thread index is always lower.
    static unsigned GetMaxThreadIndex();

    static WorkQueue* Get();


private:
    /// Process main thread tasks.
    void ProcessMainThreadTasks();
    /// Process work items until shut down. Called by the worker threads.
    void ProcessItems(unsigned threadIndex);
    /// Purge completed work items which have at least the specified priority, and send completion events as necessary.
    void PurgeCompleted(unsigned priority);
    /// Purge the pool to reduce allocation where its unneeded.
    void PurgePool();
    /// Return a work item to the pool.
    void ReturnToPool(std::shared_ptr<WorkItem>& item);
    /// Handle frame start event. Purge completed work from the main thread queue, and perform work if no threads at all.
    void HandleBeginFrame();

    /// Worker threads.
    std::vector<std::shared_ptr<WorkerThread> > threads_;
    /// Tasks to be invoked from main thread.
    WorkQueueVector<WorkFunction> mainThreadTasks_;
    /// Work item pool for reuse to cut down on allocation. The bool is a flag for item pooling and whether it is available or not.
    std::list<std::shared_ptr<WorkItem> > poolItems_;
    /// Work item collection. Accessed only by the main thread.
    std::list<std::shared_ptr<WorkItem> > workItems_;
    /// Work item prioritized queue for worker threads. Pointers are guaranteed to be valid (point to workItems).
    std::list<WorkItem*> queue_;
    /// Worker queue mutex.
    Mutex queueMutex_;
    /// Shutting down flag.
    std::atomic<bool> shutDown_;
    /// Pausing flag. Indicates the worker threads should not contend for the queue mutex.
    std::atomic<bool> pausing_;
    /// Paused flag. Indicates the queue mutex being locked to prevent worker threads using up CPU time.
    bool paused_;
    /// Completing work in the main thread flag.
    bool completing_;
    /// Tolerance for the shared pool before it begins to deallocate.
    int tolerance_;
    /// Last size of the shared pool.
    unsigned lastSize_;
    /// Maximum milliseconds per frame to spend on low-priority work, when there are no worker threads.
    int maxNonThreadedWorkMs_;
};

/// Process arbitrary array in multiple threads. Callback is copied internally.
/// One copy of callback is always used by at most one thread.
/// One copy of callback is always invoked from smaller to larger indices.
/// Signature of callback: void(unsigned beginIndex, unsigned endIndex)
template <class Callback>
void ForEachParallel(WorkQueue* workQueue, unsigned bucket, unsigned size, Callback callback)
{
    // Just call in main thread
    if (size <= bucket)
    {
        if (size > 0)
            callback(0, size);
        return;
    }

    std::atomic<unsigned> offset = 0;
    const unsigned maxThreads = workQueue->GetNumThreads() + 1;
    for (unsigned i = 0; i < maxThreads; ++i)
    {
        workQueue->AddWorkItem([=, &offset](unsigned /*threadIndex*/) mutable
                               {
                                   while (true)
                                   {
                                       const unsigned beginIndex = offset.fetch_add(bucket, std::memory_order_relaxed);
                                       if (beginIndex >= size)
                                           break;

                                       const unsigned endIndex = std::min(beginIndex + bucket, size);
                                       callback(beginIndex, endIndex);
                                   }
                               }, std::numeric_limits<unsigned>::max());
    }
    workQueue->Complete(std::numeric_limits<unsigned>::max());
}

/// Process collection in multiple threads.
/// Signature of callback: void(unsigned index, T&& element)
template <class Callback, class Collection>
void ForEachParallel(WorkQueue* workQueue, unsigned bucket, Collection&& collection, const Callback& callback)
{
    using namespace std;
    const auto collectionSize = static_cast<unsigned>(collection.size());
    ForEachParallel(workQueue, bucket, collectionSize,
        [iter = begin(collection), iterIndex = 0u, &callback](unsigned beginIndex, unsigned endIndex) mutable
    {
        iter += beginIndex - iterIndex;
        for (iterIndex = beginIndex; iterIndex < endIndex; ++iterIndex, ++iter)
            callback(iterIndex, *iter);
    });
}

/// Process collection in multiple threads with default bucket size.
template <class Callback, class Collection>
void ForEachParallel(WorkQueue* workQueue, Collection&& collection, const Callback& callback)
{
    ForEachParallel(workQueue, 1u, collection, callback);
}

/// Process collection in multiple threads.
/// Signature of callback: void(unsigned index, T&& element)
template <class Callback, class Collection>
void ForEachParallelSTD(WorkQueue* workQueue, unsigned bucket, Collection&& collection, const Callback& callback)
{
    using namespace std;
    const auto collectionSize = static_cast<unsigned>(size(collection));
    ForEachParallel(workQueue, bucket, collectionSize,
        [iter = begin(collection), iterIndex = 0u, &callback](unsigned beginIndex, unsigned endIndex) mutable
    {
        iter += beginIndex - iterIndex;
        for (iterIndex = beginIndex; iterIndex < endIndex; ++iterIndex, ++iter)
            callback(iterIndex, *iter);
    });
}

/// Process collection in multiple threads with default bucket size.
template <class Callback, class Collection>
void ForEachParallelSTD(WorkQueue* workQueue, Collection&& collection, const Callback& callback)
{
    ForEachParallelSTD(workQueue, 1u, collection, callback);
}

/// WorkQueueVector implementation
/// @{
template <class T>
void WorkQueueVector<T>::Clear()
{
    MultiVector<T>::Clear(WorkQueue::GetMaxThreadIndex());
    //this->Erase(WorkQueue::GetMaxThreadIndex());
}

template <class T>
auto WorkQueueVector<T>::Insert(const T& value)
{
    const unsigned threadIndex = WorkQueue::GetThreadIndex();
    return this->PushBack(threadIndex, value);
}

template <class T>
template <class ... Args>
T& WorkQueueVector<T>::Emplace(Args&& ... args)
{
    const unsigned threadIndex = WorkQueue::GetThreadIndex();
    return this->EmplaceBack(threadIndex, std::forward<Args>(args)...);
}
/// @}

/// Return begin iterator of const MultiVector.
template <class T> typename WorkQueueVector<T>::ConstIterator begin(const WorkQueueVector<T>& c) { return c.Begin(); }
/// Return end iterator of const MultiVector.
template <class T> typename WorkQueueVector<T>::ConstIterator end(const WorkQueueVector<T>& c) { return c.End(); }
/// Return begin iterator of mutable MultiVector.
template <class T> typename WorkQueueVector<T>::Iterator begin(WorkQueueVector<T>& c) { return c.Begin(); }
/// Return end iterator of mutable MultiVector.
template <class T> typename WorkQueueVector<T>::Iterator end(WorkQueueVector<T>& c) { return c.End(); }

}

