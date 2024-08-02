#pragma once

#include <limits>

namespace Se {

//inline const unsigned M_MAX_UNSIGNED = std::numeric_limits<unsigned>().max();

/// Update a hash with the given 8-bit value using the SDBM algorithm.
inline unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }

}