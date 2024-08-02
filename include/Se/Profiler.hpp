#pragma once

#include <cassert>

#include <Se/Console.hpp>

#define SE_API

#define SE_ASSERT(p, s, ...) SE_LOG_ERROR(s, ##__VA_ARGS__); assert(p)


#define SE_PROFILE_FUNCTION() ((void)0)

#define SE_PROFILE_SCOPE(s ...) ((void)0)



//TODO