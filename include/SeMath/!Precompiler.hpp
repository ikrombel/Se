#pragma once

#ifndef SE_MATH_STANALONE
#  include <Se/String.hpp>
#else

#include <string>

namespace Se {

typedef String std::string;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"

template<class... Args>
inline String cformat(const char * __restrict format, Args... args)
{
    int SIZE = snprintf(NULL, 0, format, args...);
    String tmp;
    tmp.resize(SIZE);
    sprintf(tmp.data(), format, args...);
    return {tmp.c_str()};
}

#pragma clang diagnostic pop

}



#endif