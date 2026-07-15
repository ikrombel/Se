#pragma once

#include <limits>
#include <unordered_set>

namespace Se {

//inline const unsigned M_MAX_UNSIGNED = std::numeric_limits<unsigned>().max();

/// Update a hash with the given 8-bit value using the SDBM algorithm.
inline unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }

template <class HashType, class ValueType>
inline void CombineHash(HashType& seed, const ValueType& v)
{
    std::hash<ValueType> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

}