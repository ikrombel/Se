#pragma once

#include <vector>

#include <algorithm>
#include <type_traits>
#include <functional>
#include <memory>

// #include <Se/String.hpp>

namespace Se {

    namespace Private {
        template <typename T>
        struct reversion_wrapper { T& iterable; };

        template <typename T>
        auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

        template <typename T>
        auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

        template <typename T>
        reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

    }

template <typename C>
inline Private::reversion_wrapper<C> Reverse(C&& c) {
    return {c};
}


template <typename Container, typename Predicate>
void EraseIf(Container& c, Predicate pred) {
    
    if (c.empty())
        return;
    
    auto endIt = c.end();
    endIt = std::next(endIt, -1);
    for (auto it = endIt; it >= c.begin(); it = std::next(it, -1))
        if (pred(*it))
            c.erase(it);

    //c.erase(RemoveIf(c.begin(), c.end(), pred), c.end());
}

template<typename T>
std::shared_ptr<T> MakeSharedArrayPtr(std::size_t size)
{
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}


// inline void StringToBuffer(std::vector<unsigned char>& dest, const char* source)
// {
//     if (!source)
//     {
//         dest.clear();
//         return;
//     }

//     unsigned size = String(source).split(' ').size();
//     dest.resize(size);

//     bool inSpace = true;
//     unsigned index = 0;
//     unsigned value = 0;

//     // Parse values
//     const char* ptr = source;
//     while (*ptr)
//     {
//         if (inSpace && *ptr != ' ')
//         {
//             inSpace = false;
//             value = (unsigned)(*ptr - '0');
//         }
//         else if (!inSpace && *ptr != ' ')
//         {
//             value *= 10;
//             value += *ptr - '0';
//         }
//         else if (!inSpace && *ptr == ' ')
//         {
//             dest[index++] = (unsigned char)value;
//             inSpace = true;
//         }

//         ++ptr;
//     }

//     // Write the final value
//     if (!inSpace && index < size)
//         dest[index] = (unsigned char)value;
// }

// inline void StringToBuffer(std::vector<unsigned char>& dest, const String& source)
// {
//     StringToBuffer(dest, source.c_str());
// }

} // namespace Se



// namespace std {
    
// template <typename T>
// struct hash<T, typename enable_if<Se::detail::is_hashable<T>::value>::type> {
//     size_t operator()(const T& res) const {
//         return res.ToHash();
//     }
// };

// } // namespace std


namespace detail
{
template<typename T, typename = void> struct is_hashable : std::false_type { };
template<typename T> struct is_hashable<T, decltype(void(&T::ToHash))> : std::true_type { };
}

// namespace std {
// template<typename T, typename = typename std::enable_if<detail::is_hashable<T>::value>::type>
// struct hash<T> {
//     size_t operator()(const T& res) const {
//         return res.ToHash();
//     }
// };
// } // namespace std

#define STATIC_INIT(unigueName, func) \
auto unigueName ## Func = func; \
auto unigueName = unigueName ## Func ();

#if 0 //Example
STATIC_INIT(Test, []() -> bool {
	return true;
});
#endif