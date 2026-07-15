// Copyright (c) 2017-2020 the rbfx project.

#pragma once

#include <memory>
#include <atomic>

namespace Se
{

/// Stop token used to thread-safely stop asynchronous task. This object can be passed
/// by value and all copies will share same internal state..
/// TODO: For better memory management split this class into StopSource with shared_ptr
/// and StopToken with weak_ptr, or reuse corresponding classes from C++20 standard library.
class StopToken
{
public:
    /// Construct default.
    StopToken() : stopped_(std::make_shared<std::atomic<bool>>(false)) {}

    /// Signal stop.
    void Stop() { *stopped_ = true; }

    /// Check whether is stopped.
    bool IsStopped() const { return *stopped_; }

private:
    /// Whether the token is stopped.
    std::shared_ptr<std::atomic<bool>> stopped_;
};

/// TODO: For better memory management
#if 0
class StopSource {
public:
    StopSource() : stopped_(std::make_shared<std::atomic<bool>>(false)) {}

    StopToken GetToken() { return StopToken(stopped_); }

    void Stop() { *stopped_ = true; }

private:
    std::shared_ptr<std::atomic<bool>> stopped_;
};

class StopToken {
public:
    StopToken(std::shared_ptr<std::atomic<bool>> stopped) : stopped_(stopped) {}

    bool IsStopped() const { return *stopped_; }

private:
    std::weak_ptr<std::atomic<bool>> stopped_;
};
#endif

}
