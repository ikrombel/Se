#pragma once

#include <cassert>

#include <Se/Console.hpp>

#define SE_API

#define SE_ASSERT(p, s, ...) SE_LOG_ERROR(s, ##__VA_ARGS__); assert(p)

#if USE_TRACY
#include <tracy/Tracy.hpp>
#  if SE_PROFILING
#    include <tracy/client/TracyLock.hpp>
//#    include <cstdint>
#    include <tracy/TracyClient.cpp>
// #    if _WIN32
// #      include <windows.h>
// #    else
// #      include "pthread.h"
// #    endif
#  endif

namespace Se
{
    static const unsigned PROFILER_COLOR_EVENTS = 0xb26d19;
    static const unsigned PROFILER_COLOR_RESOURCES = 0x006b82;

    void SetProfilerThreadName(const char* name)
    {
#  if SE_PROFILING
    tracy::SetThreadName(name);
#  endif
    }
}
#define SE_PROFILE_SCOPE(name)                  ZoneScopedN(name)
#define SE_PROFILE_FUNCTION()                   ZoneScopedN(__FUNCTION__)
#define SE_PROFILE_C(name, color)               ZoneScopedNC(name, color)
#define SE_PROFILE(name)                        ZoneScopedN(name)
#define SE_PROFILE_THREAD(name)                 Se::SetProfilerThreadName(name)
#define SE_PROFILE_VALUE(name, value)           TracyPlot(name, value)
#define SE_PROFILE_FRAME()                      FrameMark
#define SE_PROFILE_MESSAGE(txt, len)            TracyMessage(txt, len)
#define SE_PROFILE_ZONENAME(txt, len)           ZoneName(txt, len)
#  if SE_PROFILING
#   define SE_PROFILE_SRC_LOCATION(title)       [] () -> const tracy::SourceLocationData* { static const tracy::SourceLocationData srcloc { nullptr, title, __FILE__, __LINE__, 0 }; return &srcloc; }()
#   define SE_PROFILE_MUTEX(name)               ProfiledMutex name{SE_PROFILE_SRC_LOCATION_DATA(#name)}
#  else
#   define SE_PROFILE_SRC_LOCATION_DATA(title)
#   define SE_PROFILE_MUTEX(name)               Mutex name{}
#  endif
#else
#define SE_PROFILE_SCOPE(s, ...) ((void)0)
#define SE_PROFILE_FUNCTION() ((void)0)
#define SE_PROFILE_C(s, ...) ((void)0)
#define SE_PROFILE(s) ((void)0)
#define SE_PROFILE_THREAD(name) ((void)0)
#define SE_PROFILE_VALUE(name, value)  ((void)0)
#define SE_PROFILE_FRAME()  ((void)0)
#define SE_PROFILE_MESSAGE(txt, len) ((void)0)
#define SE_PROFILE_ZONENAME(s) ((void)0)
#define SE_PROFILE_SRC_LOCATION_DATA(title)
#define SE_PROFILE_MUTEX(name) Mutex name{}
#endif

//TODO