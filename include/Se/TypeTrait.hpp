#pragma once

#include <type_traits>

namespace Se
{

#ifndef SE_TYPE_TRAIT
/// Helper macro that creates type trait. Expression should be any well-formed C++ expression over template type U.
#define SE_TYPE_TRAIT(name, expr) \
    template <typename U> struct name \
    { \
        template<typename T> static decltype((expr), std::true_type{}) func(std::remove_reference<T>*); \
        template<typename T> static std::false_type func(...); \
        using type = decltype(func<U>(nullptr)); \
        static constexpr bool value{ type::value }; \
    }
#endif

}
