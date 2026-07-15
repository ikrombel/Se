#pragma once

#include <Se/Signal.hpp>

namespace Se {

// struct Application
// {
//     inline static Se::Signal<> onBeginFrame;
// };

inline String GetCurrentPlatform()
{
#if _WIN32
    return "Win32";
#elif __ANDROID__
    return "Android";
#elif __EMSCRIPTEN__
    return "Emscripten";
#elif __linux__
    return "Linux";
#else
    SE_LOG_WARNING("Unsupported platform");
    return "";
#endif
}

}