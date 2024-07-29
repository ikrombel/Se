// Copyright (c) 2022-2023 the Urho3D project.

#include "MountPoint.h"

namespace Se
{


// std::optional<FileTime> MountPoint::GetLastModifiedTime(
//     const FileIdentifier& fileName, bool creationIsModification) const
// {
//     if (Exists(fileName))
//         return 0u;
//     else
//         return std::nullopt;
// }

// String MountPoint::GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const
// {
//     return String::EMPTY;
// }

FileIdentifier MountPoint::GetIdentifierFromAbsoluteName(const String& fileFullPath) const
{
    return FileIdentifier::Empty;
}

void MountPoint::SetWatching(bool enable)
{
}

bool MountPoint::IsWatching() const
{
    return false;
}

WatchableMountPoint::WatchableMountPoint()
    : MountPoint()
{
}

WatchableMountPoint::~WatchableMountPoint() = default;

void WatchableMountPoint::SetWatching(bool enable)
{
    if (isWatching_ != enable)
    {
        isWatching_ = enable;
        if (isWatching_)
            StartWatching();
        else
            StopWatching();
    }
}

bool WatchableMountPoint::IsWatching() const
{
    return isWatching_;
}

} // namespace Se
