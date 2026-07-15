#pragma once

/// Helper to declare non-copyable class.
class NonCopyable
{
protected:
    NonCopyable() = default;

    /// Disable copy, move and assignment.
    /// @{
    NonCopyable(const NonCopyable& other) = delete;
    NonCopyable(NonCopyable && other) = delete;
    NonCopyable& operator=(const NonCopyable& other) = delete;
    NonCopyable& operator=(NonCopyable && other) = delete;
    /// @}
};

/// Helper to declare non-copyable but movable class.
class MovableNonCopyable
{
protected:
    MovableNonCopyable() = default;
    MovableNonCopyable(MovableNonCopyable && other) = default;
    MovableNonCopyable& operator=(MovableNonCopyable && other) = default;

    /// Disable copy and copy-assignment.
    /// @{
    MovableNonCopyable(const MovableNonCopyable& other) = delete;
    MovableNonCopyable& operator=(const MovableNonCopyable& other) = delete;
    /// @}
};