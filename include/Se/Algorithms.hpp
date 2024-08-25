#pragma once

#include <vector>

namespace Se {

    namespace Private {
        template <typename T>
        struct reversion_wrapper { T& iterable; };

        template <typename T>
        auto begin (reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

        template <typename T>
        auto end (reversion_wrapper<T> w) { return std::rend(w.iterable); }

        template <typename T>
        reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }

    }

template <typename C>
inline Private::reversion_wrapper<C> Reverse(C&& c) {
    return {c};
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
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