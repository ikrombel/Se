#include "MountedRoot.h"

#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>

#include <utility>

namespace Se
{

MountedRoot::MountedRoot()
    : MountPoint()
{
}

bool MountedRoot::AcceptsScheme(const String& scheme) const
{
    return scheme.comparei("file") == 0;
}

bool MountedRoot::Exists(const FileIdentifier& fileName) const
{
    if (!AcceptsScheme(fileName.scheme_))
        return false;

    auto fileSystem = FileSystem::Get();
    return IsAbsolutePath(fileName.fileName_) && fileSystem.Exists(fileName.fileName_);
}

AbstractFilePtr MountedRoot::OpenFile(const FileIdentifier& fileName, FileMode mode)
{
    if (!AcceptsScheme(fileName.scheme_))
        return nullptr;

    if (!IsAbsolutePath(fileName.fileName_))
        return nullptr;

    auto fileSystem = FileSystem::Get();

    const bool needRead = mode == FILE_READ || mode == FILE_READWRITE;
    const bool needWrite = mode == FILE_WRITE || mode == FILE_READWRITE;

    if (needRead && !fileSystem.FileExists(fileName.fileName_))
        return nullptr;

    if (needWrite)
    {
        const String directory = GetPath(fileName.fileName_);
        if (!fileSystem.DirExists(directory))
        {
            if (!fileSystem.CreateDir(directory))
                return nullptr;
        }
    }

    auto file = std::make_shared<File>(fileName.fileName_, mode);
    if (!file->IsOpen())
        return nullptr;

    file->SetName(fileName.ToUri());
    return file;
}

String MountedRoot::GetName() const
{
    static const String name = "file://";
    return name;
}

String MountedRoot::GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const
{
    if (!AcceptsScheme(fileName.scheme_))
        return String::EMPTY;

    if (IsAbsolutePath(fileName.fileName_) && FileSystem::Get().FileExists(fileName.fileName_))
        return fileName.fileName_;

    return String::EMPTY;
}

FileIdentifier MountedRoot::GetIdentifierFromAbsoluteName(const String& absoluteFileName) const
{
    return {"file", absoluteFileName};
}

void MountedRoot::Scan(
    std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    // Disable Scan for the root until we add scheme filtering.
}

} // namespace Se
