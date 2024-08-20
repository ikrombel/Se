#pragma once

#include <Se/Thread.h>
#include <Se/Timer.h>
#include <Se/String.hpp>

namespace Se {

struct Process {
    enum ProcessStatus {
        PStatusIdle,
        PStatusInProgress,
        PStatusDone,
        PStatusQuiue,
        PStatusError,
        PStatusRemove
    };

    struct Subprocess {
        String name_;
        std::function<bool()> func_;

        ProcessStatus status_;
    };

    std::vector<Subprocess> subprocesses_;
    
    volatile ProcessStatus status_{ProcessStatus::PStatusIdle};

    void AddProcess(const String& title, std::function<bool()> func);

    bool Run();

    bool IsBusy() { return status_ == ProcessStatus::PStatusInProgress; }

    void Reset() { subprocesses_.clear(); status_ = ProcessStatus::PStatusIdle; }
};

class ProcessTaskManager : public Thread {

public:
    ProcessTaskManager() : Thread() {}

    ~ProcessTaskManager() override = default;


    void ThreadFunction() override;

    void Register(const String& name, std::shared_ptr<Process> process) {        
        processes_.insert({name, process});
        if (!IsStarted())
            Run();
    }

    void SetInterprocessDelay(unsigned ms) { msDelay_ = ms; }

    static ProcessTaskManager& Get();

protected:

    bool busy_{false};

    static std::unordered_map<String, std::shared_ptr<Process>> processes_;

    String inProgress_;
    unsigned progress_{0};

    unsigned msDelay_{200};
};

}