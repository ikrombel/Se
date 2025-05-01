#pragma once

#include <algorithm>
#include <type_traits>
#include <unordered_map>

#include <Se/TypeTrait.hpp>

namespace Se {

#ifndef SEARC_TYPE_TRAIT
/// Helper macro that creates type trait. Expression should be any well-formed C++ expression over template type U.
#define SEARC_TYPE_TRAIT(name, expr) \
    template <typename U> struct name \
    { \
        template<typename T> static decltype((expr), std::true_type{}) func(std::remove_reference<T>*); \
        template<typename T> static std::false_type func(...); \
        using type = decltype(func<U>(nullptr)); \
        static constexpr bool value{ type::value }; \
    }
#endif

SEARC_TYPE_TRAIT(IsToHash, std::declval<T&>().ToHash());

//typedef std::size_t Hash;
typedef unsigned Hash;

/// Combine hash into result value.
template <class T>
inline void hash_combine(T& result, Hash hash, std::enable_if_t<sizeof(T) == 4, int>* = 0)
{
    result ^= hash + 0x9e3779b9 + (result << 6) + (result >> 2);
}

template <class T>
inline void hash_combine(T& result, unsigned long long hash, std::enable_if_t<sizeof(T) == 8, int>* = 0)
{
    result ^= hash + 0x9e3779b97f4a7c15ull + (result << 6) + (result >> 2);
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/// Fold 64-bit hash to 32-bit.
inline Hash fold_hash(unsigned long long value)
{
    const auto lowValue = static_cast<unsigned>(value);
    const auto highValue = static_cast<unsigned>(value >> 32ull);
    if (highValue == 0)
        return lowValue;

    Hash result = lowValue;
    hash_combine(result, highValue);
    return result;
}

/// Make hash template helper.
template <class T>
inline Hash make_hash(const T& value)
{
    Hash hash{};

    if constexpr (IsToHash<T>::value)
        hash = value.ToHash();
    else
        hash = fold_hash(std::hash<T>()(value));
    
    return hash;
}


template<typename Value>
inline Hash make_hash(const std::vector<Value>& vec) {
    Hash hash = 0;
    for (const auto& v : vec) {
        hash_combine(hash, make_hash(v));
    }
    return hash;
}

template<typename Key, typename Value>
inline Hash make_hash(const std::unordered_map<Key, Value>& map) {
    Hash hash = 0;
    for (const auto& pair : map) {
        //hash ^= make_hash(pair.first) ^ make_hash(pair.second);
        hash_combine(hash, make_hash(pair.first));
        hash_combine(hash, make_hash(pair.second));
    }
    return hash;
}

}
