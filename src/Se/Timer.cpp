
#include "Timer.h"

//#include <GFrost/Core/CoreEvents.h>
#include <Se/Profiler.hpp>

#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#elif __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Se
{

bool HiresTimer::supported(false);
long long HiresTimer::frequency(1000);


Signal<const TimeParams&> Time::onBeginFrame;
Signal<> Time::onEndFrame;
Signal<> Time::onEndFramePlugin;

Time::Time() :
    frameNumber_(0),
    timeStep_(0.0f),
    timerPeriod_(0)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency))
    {
        HiresTimer::frequency = frequency.QuadPart;
        HiresTimer::supported = true;
    }
#else
    HiresTimer::frequency = 1000000;
    HiresTimer::supported = true;
#endif
}

Time::~Time()
{
    SetTimerPeriod(0);
}

static unsigned Tick()
{
#ifdef _WIN32
    return (unsigned)timeGetTime();
#elif __EMSCRIPTEN__
    return (unsigned)emscripten_get_now();
#else
    struct timeval time{};
    gettimeofday(&time, nullptr);
    return (unsigned)(time.tv_sec * 1000 + time.tv_usec / 1000);
#endif
}

static long long HiresTick()
{
#ifdef _WIN32
    if (HiresTimer::IsSupported())
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }
    else
        return timeGetTime();
#elif __EMSCRIPTEN__
    return (long long)(emscripten_get_now()*1000.0);
#else
    struct timeval time{};
    gettimeofday(&time, nullptr);
    return time.tv_sec * 1000000LL + time.tv_usec;
#endif
}

void Time::BeginFrame(float timeStep)
{
    ++frameNumber_;
    if (!frameNumber_)
        ++frameNumber_;

    timeStep_ = timeStep;

    {
        SE_PROFILE("BeginFrame");

        onBeginFrame({frameNumber_, timeStep_});
    }
}

void Time::EndFrame()
{
    {
        SE_PROFILE("EndFrame");

        // Frame end event
        onEndFrame();

        // Internal frame end event used only by the engine/tools 
        // E_ENDFRAMEPLUGIN (old E_ENDFRAMEPRIVATE);
        onEndFramePlugin();

    }
}

void Time::SetTimerPeriod(unsigned mSec)
{
#ifdef _WIN32
    if (timerPeriod_ > 0)
        timeEndPeriod(timerPeriod_);

    timerPeriod_ = mSec;

    if (timerPeriod_ > 0)
        timeBeginPeriod(timerPeriod_);
#endif
}

float Time::GetElapsedTime()
{
    return elapsedTime_.GetMSec(false) / 1000.0f;
}

unsigned Time::GetSystemTime()
{
    return Tick();
}

unsigned Time::GetTimeSinceEpoch()
{
    return (unsigned)time(nullptr);
}

//String Time::GetTimeStamp()
//{
//    time_t sysTime;
//    time(&sysTime);
//    const char* dateTime = ctime(&sysTime);
//    return String(dateTime).Replaced("\n", "");
//}

String Time::GetTimeStamp(const char* format)
{
    time_t timestamp = 0;
    time(&timestamp);
    return GetTimeStamp(timestamp, format);
}

String Time::GetTimeStamp(time_t timestamp, const char* format)
{
    if (format == nullptr)
        format = DEFAULT_DATE_TIME_FORMAT;

    char dateTime[128];
    tm* timeInfo = localtime(&timestamp);
    strftime(dateTime, sizeof(dateTime), format, timeInfo);
    return dateTime;
}

void Time::Sleep(unsigned mSec)
{
#ifdef _WIN32
    ::Sleep(mSec);
#else
    timespec time{static_cast<time_t>(mSec / 1000), static_cast<long>((mSec % 1000) * 1000000)};
    nanosleep(&time, nullptr);
#endif
}

float Time::GetFramesPerSecond() const
{
    return 1.0f / timeStep_;
}


Time& Time::Get()
{
    static Time* ptr_;
    if (!ptr_) {
        //SE_LOG_INFO("Time initialized.");
        ptr_ = new Time();
    }
    return *ptr_;
}


Timer::Timer()
{
    Reset();
}

unsigned Timer::GetMSec(bool reset)
{
    unsigned currentTime = Tick();
    unsigned elapsedTime = currentTime - startTime_;
    if (reset)
        startTime_ = currentTime;

    return elapsedTime;
}

void Timer::Reset()
{
    startTime_ = Tick();
}

HiresTimer::HiresTimer()
{
    Reset();
}

long long HiresTimer::GetUSec(bool reset)
{
    long long currentTime = HiresTick();
    long long elapsedTime = currentTime - startTime_;

    // Correct for possible weirdness with changing internal frequency
    if (elapsedTime < 0)
        elapsedTime = 0;

    if (reset)
        startTime_ = currentTime;

    return (elapsedTime * 1000000LL) / frequency;
}

void HiresTimer::Reset()
{
    startTime_ = HiresTick();
}


long long Time::getTime()
{
    static int init = 0;
    static long long offset = 0;
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    LARGE_INTEGER tval;
    if(init == 0) {
        init = 1;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&tval);
        offset = (long long)tval.QuadPart * 1000000 / frequency.QuadPart;
    }
    QueryPerformanceCounter(&tval);
    return (long long)tval.QuadPart * 1000000 / frequency.QuadPart - offset;
#elif __linux__
    struct timeval tval;
    if(init == 0) {
        init = 1;
        gettimeofday(&tval,0);
        offset = (long long)tval.tv_sec * 1000000 + tval.tv_usec;
    }
    gettimeofday(&tval,0);
    return (long long)tval.tv_sec * 1000000 + tval.tv_usec - offset;
#elif _CELLOS_LV2
    sys_time_sec_t seconds;
    sys_time_nsec_t nseconds;
    if(init == 0) {
        init = 1;
        sys_time_get_current_time(&seconds,&nseconds);
        offset = (long long)seconds * 1000000 + nseconds / 1000;
    }
    sys_time_get_current_time(&seconds,&nseconds);
    return (long long)seconds * 1000000 + nseconds / 1000 - offset;
#else
    return 0;
#endif
}

}
