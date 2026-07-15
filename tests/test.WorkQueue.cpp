#include <Se/Console.hpp>
#include <Se/WorkQueue.h>

struct MyWork: public Se::WorkItem
{
    int jobIndex_;
    virtual void Work(unsigned threadIndex)
    {
        // Simulate work...
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 + (jobIndex_ * 500)));
        // Store result (not thread-safe in general, but fine here)
        result_ = jobIndex_ * 2;

        std::this_thread::sleep_for(std::chrono::milliseconds( (jobIndex_ * 500)));

        result_ = jobIndex_ * 2+1;
    }
    int result_ = 0;
};

Se::WorkQueue* queue = Se::WorkQueue::Get();
std::vector<std::shared_ptr<MyWork>> jobs;


//if (ImGui::Button("Run WorkQueue"))
void TestWorkQueue()
{
    queue = Se::WorkQueue::Get();

    // Prepare some jobs
    const int numJobs = 5;
    
    for (int i = 0; i < numJobs; ++i)
    {
        auto work = std::make_shared<MyWork>();
        work->workFunction_ = [](const Se::WorkItem* item, unsigned threadIndex)
        {
            MyWork* w = const_cast<MyWork*>(static_cast<const MyWork*>(item));
            w->Work(threadIndex);
        };
        work->priority_ = 0;
        work->jobIndex_ = i;
        queue->AddWorkItem(work);
        jobs.push_back(work);
    }

    // Wait for all jobs to finish
    //queue->Complete(0);

    queue->onWorkCompleted.connect([]()
    {
        SE_LOG_ERROR("Job completed on main thread");
        
    });
    queue->CreateThreads(2);
    queue->Resume(); //Run
}