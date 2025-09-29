#include "Thread.h"

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif

#include <Se/String.hpp>

namespace Se
{

#ifdef SE_THREADING
#  ifdef _WIN32

#if !defined(UWP)
typedef HRESULT (WINAPI *pfnSetThreadDescription)(HANDLE, PCWSTR);
static const auto pSetThreadDescription = (pfnSetThreadDescription) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetThreadDescription");
#endif

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; /* must be 0x1000 */
    LPCSTR szName; /* pointer to name (in user addr space) */
    DWORD dwThreadID; /* thread ID (-1=caller thread) */
    DWORD dwFlags; /* reserved for future use, must be zero */
} THREADNAME_INFO;
#pragma pack(pop)


typedef HRESULT (WINAPI *pfnSetThreadDescription)(HANDLE, PCWSTR);

static DWORD WINAPI ThreadFunctionStatic(LPVOID data)
{
    Thread* thread = static_cast<Thread*>(data);
#if !defined(UWP)
    if (pSetThreadDescription)
        pSetThreadDescription(GetCurrentThread(), MultiByteToWide(thread->GetName().c_str()).c_str());
    else
#endif

    thread->ThreadFunction();
    return 0;
}

#  else

void* Thread::ThreadFunctionStatic(void* data)
{
    auto* thread = static_cast<Thread*>(data);

#if defined(__ANDROID_API__)
#  if __ANDROID_API__ < 22
    prctl(PR_SET_NAME, thread->name_.c_str(), 0, 0, 0);
#  else
    pthread_setname_np(pthread_self(), thread->name_.c_str());
#  endif
#elif defined(__linux__)
    pthread_setname_np(pthread_self(), thread->name_.c_str());
#elif defined(__MACOSX__) || defined(__IPHONEOS__)
    pthread_setname_np(thread->name_.c_str());
#endif

    thread->ThreadFunction();
    pthread_exit((void*)nullptr);
    return nullptr;
}

#  endif
#endif // SE_THREADING

ThreadID Thread::mainThreadID;

Thread::Thread(const std::string& name) :
    handle_(nullptr),
    shouldRun_(false),
    name_(name)
{
}

Thread::~Thread()
{
    Stop();
}

bool Thread::Run()
{
#ifdef SE_THREADING
    // Check if already running
    if (handle_)
        return false;

    shouldRun_ = true;
#  ifdef _WIN32
    handle_ = CreateThread(nullptr, 0, Se::ThreadFunctionStatic, (void*)this, 0, nullptr);
#  else
    handle_ = new pthread_t;
    pthread_attr_t type;
    pthread_attr_init(&type);
    pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
    pthread_create((pthread_t*)handle_, &type, &Thread::ThreadFunctionStatic, this);
#  endif
    return handle_ != nullptr;
#else
    return false;
#endif // SE_THREADING
}

void Thread::Stop()
{
#ifdef SE_THREADING
    // Check if already stopped
    if (!handle_)
        return;

    shouldRun_ = false;
#  ifdef _WIN32
    WaitForSingleObject((HANDLE)handle_, INFINITE);
    CloseHandle((HANDLE)handle_);
#  else
    auto* thread = (pthread_t*)handle_;
    if (thread)
        pthread_join(*thread, nullptr);
    delete thread;
#  endif
    handle_ = nullptr;
#endif // SE_THREADING
}

void Thread::SetPriority(int priority)
{
#ifdef SE_THREADING
#  ifdef _WIN32
    if (handle_)
        SetThreadPriority((HANDLE)handle_, priority);
#  elif defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    auto* thread = (pthread_t*)handle_;
    if (thread)
        pthread_setschedprio(*thread, priority);
#  endif
#endif // SE_THREADING
}

void Thread::SetMainThread()
{
    mainThreadID = GetCurrentThreadID();
}

ThreadID Thread::GetCurrentThreadID()
{
#ifdef SE_THREADING
#  ifdef _WIN32
    return GetCurrentThreadId();
#  else
    return pthread_self();
#  endif
#else
    return ThreadID();
#endif // SE_THREADING
}

bool Thread::IsMainThread()
{
#ifdef SE_THREADING
    return GetCurrentThreadID() == mainThreadID;
#else
    return true;
#endif // SE_THREADING
}

int AtomicCAS(volatile int *ptr,int old_value,int new_value) {
#ifdef _WIN32
    return (_InterlockedCompareExchange((long volatile*)ptr, new_value, old_value) == old_value);
#elif __linux__
    return (__sync_val_compare_and_swap(ptr,old_value,new_value) == old_value);
#elif _CELLOS_LV2
    return (cellAtomicCompareAndSwap32((uint32_t*)ptr,(uint32_t)old_value,(uint32_t)new_value) == (uint32_t)old_value);
#else
    if (*ptr != old_value)
        return 0;
    *ptr = new_value;
    return 1;
#endif
}

void SpinLock(volatile int *ptr,int old_value,int new_value) {
    while(!AtomicCAS(ptr, old_value, new_value));
}

void WaitLock(volatile int *ptr,int value) {
    while(!AtomicCAS(ptr, value, value));
}

void Thread::SetName(const std::string& name)
{
    if (handle_ != nullptr)
    {
        SE_LOG_ERROR("Thread name must be set before thread is started.");
        return;
    }
    name_ = name;
}

}
