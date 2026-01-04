
//#include <GFrost/Core/CoreEvents.h>
//#include <GFrost/Core/ProcessUtils.h>
//#include <Se/WorkQueue.h>
#include "WorkQueue.h"

#include <Se/Profiler.hpp>

#include <Se/Console.hpp>
#include <Se/Thread.h>
#include <Se/Timer.h>

#include <Se/Platform/InitFPU.hpp>

namespace Se
{

/// Thread index.
static thread_local unsigned currentThreadIndex = std::numeric_limits<unsigned>::max();
static unsigned maxThreadIndex = 1;

/// Worker thread managed by the work queue.
class WorkerThread : public Thread //, public RefCounted
{
public:
    /// Construct.
    WorkerThread(WorkQueue* owner, unsigned index) :
        owner_(owner),
        index_(index)
    {
    }

    /// Process work items until stopped.
    void ThreadFunction() override
    {
        SE_PROFILE_THREAD(format("WorkerThread {}", (uint64_t)GetCurrentThreadID()));
        currentThreadIndex = index_;
        // Init FPU state first
        InitFPU();
        owner_->ProcessItems(index_);
    }

    /// Return thread index.
    unsigned GetIndex() const { return index_; }

private:
    /// Work queue.
    WorkQueue* owner_;
    /// Thread index.
    unsigned index_;
};

WorkQueue::WorkQueue() :
    shutDown_(false),
    pausing_(false),
    paused_(false),
    completing_(false),
    tolerance_(10),
    lastSize_(0),
    maxNonThreadedWorkMs_(5)
{
    currentThreadIndex = 0;
    maxThreadIndex = 1;
    mainThreadTasks_.Clear();
    Time::onBeginFrame.connect([this](const TimeParams&){
            HandleBeginFrame();
    });
}

WorkQueue::~WorkQueue()
{
    // Stop the worker threads. First make sure they are not waiting for work items
    shutDown_ = true;
    Resume();

    for (auto i = 0; i < threads_.size(); ++i)
        threads_[i]->Stop();
}

void WorkQueue::CreateThreads(unsigned numThreads)
{
#ifdef SE_THREADING
    // Other subsystems may initialize themselves according to the number of threads.
    // Therefore allow creating the threads only once, after which the amount is fixed
    if (!threads_.empty())
        return;

    // Start threads in paused mode
    Pause();

    for (auto i = 0; i < numThreads; ++i)
    {
        auto thread = std::make_shared<WorkerThread>(this, i + 1);
        thread->SetName(format("Worker {}", i + 1));
        thread->Run();
        threads_.push_back(thread);
    }
    mainThreadTasks_.Clear();
#else
    SE_LOG_ERROR("Can not create worker threads as threading is disabled");
#endif
}

void WorkQueue::CallFromMainThread(WorkFunction workFunction)
{
    if (GetThreadIndex() == 0)
    {
        workFunction(0);
        return;
    }

    mainThreadTasks_.Insert(std::move(workFunction));
}

std::shared_ptr<WorkItem> WorkQueue::GetFreeItem()
{
    if (!poolItems_.empty())
    {
        std::shared_ptr<WorkItem> item = poolItems_.front();
        poolItems_.pop_front();
        return item;
    }
    else
    {
        // No usable items found, create a new one set it as pooled and return it.
        auto item = std::make_shared<WorkItem>();
        item->pooled_ = true;
        return item;
    }
}

void WorkQueue::AddWorkItem(const std::shared_ptr<WorkItem>& item)
{
    if (!item)
    {
        SE_LOG_ERROR("Null work item submitted to the work queue");
        return;
    }

    // Check for duplicate items.
    assert(std::find(workItems_.begin(), workItems_.end(), item) == workItems_.end());

    // Push to the main thread list to keep item alive
    // Clear completed flag in case item is reused
    workItems_.push_back(item);
    item->completed_ = false;

    // Make sure worker threads' list is safe to modify
    if (threads_.size() && !paused_)
        queueMutex_.Acquire();

    // Find position for new item
    if (queue_.empty())
        queue_.push_back(item.get());
    else
    {
        bool inserted = false;

        for (auto i = queue_.begin(); i != queue_.end(); ++i)
        {
            if ((*i)->priority_ <= item->priority_)
            {
                queue_.insert(i, item.get());
                inserted = true;
                break;
            }
        }

        if (!inserted)
            queue_.push_back(item.get());
    }

    if (threads_.size())
    {
        queueMutex_.Release();
        paused_ = false;
    }
}

std::shared_ptr<WorkItem> WorkQueue::AddWorkItem(WorkFunction workFunction, unsigned priority)
{
    std::shared_ptr<WorkItem> item = GetFreeItem();
    item->workLambda_ = std::move(workFunction);
    item->workFunction_ = [](const WorkItem* item, unsigned threadIndex) { item->workLambda_(threadIndex); };
    item->priority_ = priority;
    AddWorkItem(item);
    return item;
}

bool WorkQueue::RemoveWorkItem(std::shared_ptr<WorkItem> item)
{
    if (!item)
        return false;

    MutexLock lock(queueMutex_);

    // Can only remove successfully if the item was not yet taken by threads for execution
    auto i = std::find(queue_.begin(), queue_.end(), item.get());
    if (i != queue_.end())
    {
        auto j = std::find(workItems_.begin(), workItems_.end(), item);
        if (j != workItems_.end())
        {
            queue_.erase(i);
            ReturnToPool(item);
            workItems_.erase(j);
            return true;
        }
    }

    return false;
}

unsigned WorkQueue::RemoveWorkItems(const std::vector<std::shared_ptr<WorkItem> >& items)
{
    MutexLock lock(queueMutex_);
    unsigned removed = 0;

    for (auto i = items.begin(); i != items.end(); ++i)
    {
        auto j = std::find(queue_.begin(), queue_.end(), i->get());
        if (j != queue_.end())
        {
            auto k = std::find(workItems_.begin(), workItems_.end(), *i); // workItems_.Find(*i);
            if (k != workItems_.end())
            {
                queue_.erase(j);
                ReturnToPool(*k);
                workItems_.erase(k);
                ++removed;
            }
        }
    }

    return removed;
}

void WorkQueue::Pause()
{
    if (!paused_)
    {
        pausing_ = true;

        queueMutex_.Acquire();
        paused_ = true;

        pausing_ = false;
    }
}

void WorkQueue::Resume()
{
    if (paused_)
    {
        queueMutex_.Release();
        paused_ = false;
    }
}


void WorkQueue::Complete(unsigned priority)
{
    completing_ = true;

    if (threads_.size())
    {
        Resume();

        // Take work items also in the main thread until queue empty or no high-priority items anymore
        while (!queue_.empty())
        {
            queueMutex_.Acquire();
            if (!queue_.empty() && queue_.front()->priority_ >= priority)
            {
                WorkItem* item = queue_.front();
                queue_.pop_front();
                queueMutex_.Release();
                item->workFunction_(item, 0);
                item->completed_ = true;
            }
            else
            {
                queueMutex_.Release();
                break;
            }
        }

        // Wait for threaded work to complete
        while (!IsCompleted(priority))
        {
        }

        // If no work at all remaining, pause worker threads by leaving the mutex locked
        if (queue_.empty())
            Pause();
    }
    else
    {
        // No worker threads: ensure all high-priority items are completed in the main thread
        while (!queue_.empty() && queue_.front()->priority_ >= priority)
        {
            WorkItem* item = queue_.front();
            queue_.pop_front();
            item->workFunction_(item, 0);
            item->completed_ = true;
        }
    }

    PurgeCompleted(priority);
    completing_ = false;

    ProcessMainThreadTasks();
}

unsigned WorkQueue::GetNumIncomplete(unsigned priority) const
{
    unsigned incomplete = 0;
    for (const auto& workItem : workItems_)
    {
        if (workItem->priority_ >= priority && !workItem->completed_)
            ++incomplete;
    }

    return incomplete;
}

bool WorkQueue::IsCompleted(unsigned priority) const
{
    for (const auto & workItem : workItems_)
    {
        if (workItem->priority_ >= priority && !workItem->completed_)
            return false;
    }

    return true;
}

void WorkQueue::ProcessMainThreadTasks()
{
    for (auto callback : mainThreadTasks_)
        callback(0);
    mainThreadTasks_.Clear();
}

void WorkQueue::ProcessItems(unsigned threadIndex)
{
    bool wasActive = false;

    for (;;)
    {
        if (shutDown_)
            return;

        if (pausing_ && !wasActive)
            Time::Sleep(0);
        else
        {
            queueMutex_.Acquire();
            if (!queue_.empty())
            {
                wasActive = true;

                WorkItem* item = queue_.front();
                queue_.pop_front();
                queueMutex_.Release();
                item->workFunction_(item, threadIndex);
                item->completed_ = true;
            }
            else
            {
                wasActive = false;
                if (!onWorkCompleted.empty() && this->IsCompleted(0)) {
                    onWorkCompleted();
                    onWorkCompleted.disconnectAll();
                }
                
                queueMutex_.Release();
                Time::Sleep(0);
            }
        }
    }
}

void WorkQueue::PurgeCompleted(unsigned priority)
{
    // Purge completed work items and send completion events. Do not signal items lower than priority threshold,
    // as those may be user submitted and lead to eg. scene manipulation that could happen in the middle of the
    // render update, which is not allowed
    for (auto i = workItems_.begin(); i != workItems_.end();)
    {
        if ((*i)->completed_ && (*i)->priority_ >= priority)
        {
            if ((*i)->sendEvent_)
            {
                // E_WORKITEMCOMPLETED
                if (*i)
                    onWorkItemCompleted(i->get());
            }

            ReturnToPool(*i);
            i = workItems_.erase(i);
        }
        else
            ++i;
    }
}

void WorkQueue::PurgePool()
{
    std::size_t currentSize = poolItems_.size();
    std::size_t difference = lastSize_ - currentSize;

    // Difference tolerance, should be fairly significant to reduce the pool size.
    for (std::size_t i = 0; poolItems_.size() > 0 && difference > tolerance_ && i < (unsigned)difference; i++)
        poolItems_.pop_front();

    lastSize_ = currentSize;
}

void WorkQueue::ReturnToPool(std::shared_ptr<WorkItem>& item)
{
    // Check if this was a pooled item and set it to usable
    if (item->pooled_)
    {
        // Reset the values to their defaults. This should
        // be safe to do here as the completed event has
        // already been handled and this is part of the
        // internal pool.
        item->start_ = nullptr;
        item->end_ = nullptr;
        item->aux_ = nullptr;
        item->workFunction_ = nullptr;
        item->priority_ = std::numeric_limits<unsigned>::max();
        item->sendEvent_ = false;
        item->completed_ = false;

        poolItems_.push_back(item);
    }
}

void WorkQueue::HandleBeginFrame()
{
    ProcessMainThreadTasks();

    // If no worker threads, complete low-priority work here
    if (threads_.empty() && !queue_.empty())
    {
        SE_PROFILE("CompleteWorkNonthreaded");

        HiresTimer timer;

        while (!queue_.empty() && timer.GetUSec(false) < maxNonThreadedWorkMs_ * 1000LL)
        {
            WorkItem* item = queue_.front();
            queue_.pop_front();
            item->workFunction_(item, 0);
            item->completed_ = true;
        }

        //onWorkCompleted();
    }

    // Complete and signal items down to the lowest priority
    PurgeCompleted(0);
    PurgePool();
}

unsigned WorkQueue::GetThreadIndex()
{
    return currentThreadIndex;
}

unsigned WorkQueue::GetMaxThreadIndex()
{
    return maxThreadIndex;
}

WorkQueue* WorkQueue::Get()
{
    static WorkQueue* ret;

    if (!ret)
        ret = new WorkQueue();

    return ret;
}

}
