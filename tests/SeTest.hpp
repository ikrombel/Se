#pragma once

#include <Se/Console.hpp>

#define SE_ASSERT(x) { \
    auto cond = x; \
    if (!(x)) { \
        SE_LOG_ERROR("Assertion failed: {}", #x); assert(x); \
    } \
}