#include "Console.hpp"

#include "ProcessTaskManager.h"

namespace Se {

void ProcessTaskManager::ThreadFunction()
{
    using ProcessStatus = Process::ProcessStatus;

    while(!processes_.empty())
    {
        std::shared_ptr<Process> activeProcess;

        if (inProgress_.empty()) {
            Time::Sleep(1000);

            for (auto it = processes_.begin(); it != processes_.end(); ++it) {
                auto& process = it->second;

                switch (process->status_)
                {
                case ProcessStatus::PStatusQuiue:
                    inProgress_ = it->first;
                    process->status_ = ProcessStatus::PStatusInProgress;
                    for(auto& subprocess : process->subprocesses_)
                        subprocess.status_ = ProcessStatus::PStatusQuiue;
                    busy_ = true;
                    activeProcess = process;
                    break;
                case ProcessStatus::PStatusRemove:
                    process->Reset();
                    break;
                
                default:
                    break;
                }
            }
        }

        if (activeProcess) {
            for (auto& subproc : activeProcess->subprocesses_) {

                subproc.status_ = ProcessStatus::PStatusInProgress;
                bool isOk = subproc.func_();
                if (!isOk && subproc.onTerminate){
                    subproc.onTerminate();
                    break;
                }
                subproc.status_ = isOk ? ProcessStatus::PStatusDone : ProcessStatus::PStatusError;
                progress_++;

                Time::Sleep(msDelay_);
            }

            busy_ = false;
            inProgress_.clear();
            activeProcess->Reset();
        }
    }
}

void Process::AddProcess(const String& title, std::function<bool()> func, 
        std::function<void()> onTerminate)
{
    subprocesses_.push_back({title, func, ProcessStatus::PStatusIdle, onTerminate});
}


bool Process::Run() {
    if (subprocesses_.empty() || IsBusy())
        return false;

    status_ = ProcessStatus::PStatusQuiue;
    return true;
}

ProcessTaskManager& ProcessTaskManager::Get()
{
    static ProcessTaskManager* ptr_;
    if (!ptr_) {
        SE_LOG_INFO("ProcessTaskManager initialized.");
        ptr_ = new ProcessTaskManager();
    }
    return *ptr_;
}

}