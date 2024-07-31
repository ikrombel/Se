#pragma once

#include <Se/Signal.hpp>

namespace Se {

struct Application
{
    inline static Se::Signal<> onBeginFrame;
};

}