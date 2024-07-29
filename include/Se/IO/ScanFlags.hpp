#pragma once

#include <Se/FlagSet.hpp>

namespace Se
{

enum ScanFlag : unsigned char
{
    SCAN_FILES = 0x1,
    SCAN_DIRS = 0x2,
    SCAN_HIDDEN = 0x4,
    SCAN_APPEND = 0x8,
    SCAN_RECURSIVE = 0x10,
};
SE_FLAGSET(ScanFlag, ScanFlags);

/// Alias for type used for file times.
/// TODO(editor): Make 64 bit?
using FileTime = unsigned;

}
