#pragma once

#include <thread>
#include <mutex>
//#include <condition_variable>

namespace Se {

/// Wrapper for the C++ std::mutex.
using Mutex = std::mutex;

/// Wrapper for the C++ std::recursive_mutex.
using RecursiveMutex = std::recursive_mutex;

// /// Wrapper for the C++ std::condition_variable.
// using Signal = std::condition_variable;

// /// Wrapper for the C++ std::thread.
// using Thread = std::thread;

// /// Wrapper for the C++ std::thread::id.
// using ThreadId = std::thread::id;

/// Wrapper for the C++ std::unique_lock<std::mutex>.
using Lock = std::unique_lock<Mutex>;

/// Wrapper for the C++ std::unique_lock<std::recursive_mutex>.
using RecursiveLock = std::unique_lock<RecursiveMutex>;

using LockGuard = std::lock_guard<Mutex>;
using RecursiveLockGuard = std::lock_guard<RecursiveMutex>;

}