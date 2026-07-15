

#pragma once


#ifndef _WIN32
#include <pthread.h>
using ThreadID = pthread_t;
#else
using ThreadID = unsigned;
#endif

#include <Se/Mutex.hpp>

#include <chrono>


namespace Se
{

/// Operating system thread.
class Thread
{
public:
    /// Construct. Does not start the thread yet.
    Thread(const std::string& mame = "");
    /// Destruct. If running, stop and wait for thread to finish.
    virtual ~Thread();

    /// The function to run in the thread.
    virtual void ThreadFunction() = 0;

    /// Start running the thread. Return true if successful, or false if already running or if can not create the thread.
    bool Run();
    /// Set the running flag to false and wait for the thread to finish.
    void Stop();
    /// Set thread priority. The thread must have been started first.
    void SetPriority(int priority);

    /// Return whether thread exists.
    bool IsStarted() const { return handle_ != nullptr; }
    /// Set name of the platform thread on supported platforms. Must be called before Run().
    void SetName(const std::string& name);

    /// Set the current thread as the main thread.
    static void SetMainThread();
    /// Return the current thread's ID.
    static ThreadID GetCurrentThreadID();
    /// Return whether is executing in the main thread.
    static bool IsMainThread();

#if _WIN32
    static DWORD ThreadFunctionStatic(LPVOID* data);
#else
    static void* ThreadFunctionStatic(void* data);
#endif

    std::string GetName() const { return name_; }

protected:
    /// Name of the thread. It will be propagated to underlying OS thread if possible.
    std::string name_{};
    /// Thread handle.
    void* handle_;
    /// Running flag.
    volatile bool shouldRun_;

    /// Main thread's thread ID.
    static ThreadID mainThreadID;
};

int AtomicCAS(volatile int *ptr, int old_value, int new_value);

void SpinLock(volatile int *ptr, int old_value, int new_value);

void WaitLock(volatile int *ptr, int value);

class AtomicLock {

public:

    AtomicLock(volatile int *ptr) : ptr(ptr) {
        SpinLock(ptr, 0, 1);
    }
    ~AtomicLock() {
        SpinLock(ptr, 1, 0);
    }

private:

    volatile int *ptr;
};

}
