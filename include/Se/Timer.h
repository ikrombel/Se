

#pragma once

#include <Se/String.hpp>
#include <Se/Signal.hpp>

namespace Se
{

static const char* DEFAULT_DATE_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

/// Low-resolution operating system timer.
class Timer
{
public:
    /// Construct. Get the starting clock value.
    Timer();

    /// Return elapsed milliseconds and optionally reset.
    unsigned GetMSec(bool reset);
    /// Reset the timer.
    void Reset();

private:
    /// Starting clock value in milliseconds.
    unsigned startTime_{};
};

/// High-resolution operating system timer used in profiling.
class HiresTimer
{
    friend class Time;

public:
    /// Construct. Get the starting high-resolution clock value.
    HiresTimer();

    /// Return elapsed microseconds and optionally reset.
    long long GetUSec(bool reset);
    /// Reset the timer.
    void Reset();

    /// Return if high-resolution timer is supported.
    static bool IsSupported() { return supported; }

    /// Return high-resolution timer frequency if supported.
    static long long GetFrequency() { return frequency; }

private:
    /// Starting clock value in CPU ticks.
    long long startTime_{};

    /// High-resolution timer support flag.
    static bool supported;
    /// High-resolution timer frequency.
    static long long frequency;
};


struct TimeParams {
    /// Frame number.
    unsigned frameNumber;
    /// Timestep in seconds.
    float timeStep;
};

/// %Time and frame counter subsystem.
class Time
{
public:
    Signal<const TimeParams&> onBeginFrame;
    Signal<> onEndFrame;


    enum {
        CLOCKS_PER_SECOND = 1000000,
    };

    /// Construct.
    explicit Time();
    /// Destruct. Reset the low-resolution timer period if set.
    virtual ~Time();

    /// Begin new frame, with (last) frame duration in seconds and send frame start event.
    void BeginFrame(float timeStep);
    /// End frame. Increment total time and send frame end event.
    void EndFrame();
    /// Set the low-resolution timer period in milliseconds. 0 resets to the default period.
    void SetTimerPeriod(unsigned mSec);

    /// Return frame number, starting from 1 once BeginFrame() is called for the first time.
    unsigned GetFrameNumber() const { return frameNumber_; }

    /// Return current frame timestep as seconds.
    float GetTimeStep() const { return timeStep_; }

    /// Return current low-resolution timer period in milliseconds.
    unsigned GetTimerPeriod() const { return timerPeriod_; }

    /// Return elapsed time from program start as seconds.
    float GetElapsedTime();

    /// Return current frames per second.
    float GetFramesPerSecond() const;

    /// Get system time as milliseconds.
    static unsigned GetSystemTime();
    /// Get system time as seconds since 1.1.1970.
    static unsigned GetTimeSinceEpoch();
    /// Get a date/time stamp as a string.
    static String GetTimeStamp(const char* format=nullptr);
    /// Get a date/time stamp as a string.
    static String GetTimeStamp(time_t timestamp, const char* format=nullptr);
    /// Sleep for a number of milliseconds.
    static void Sleep(unsigned mSec);
    /// return current time in microseconds
    static long long getTime();

private:
    /// Elapsed time since program start.
    Timer elapsedTime_;
    /// Frame number.
    unsigned frameNumber_;
    /// Timestep in seconds.
    float timeStep_;
    /// Low-resolution timer period.
    unsigned timerPeriod_;
};

}
