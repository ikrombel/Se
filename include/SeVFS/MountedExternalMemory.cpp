// Copyright (c) 2023-2023 the rbfx project.

#include "MountedExternalMemory.h"

#include <Se/IO/FileSystem.h>

//#include "Urho3D/Resource/ResourceEvents.h"

#include <utility>

namespace Se
{

namespace
{

class WrappedMemoryBuffer : public MemoryBuffer
{
public:
    WrappedMemoryBuffer(const MemoryBuffer& buffer)
        : MemoryBuffer(buffer.GetData(), buffer.GetSize())
    {
    }
};

}

MountedExternalMemory::MountedExternalMemory(const String& scheme)
    : MountPoint()
    , scheme_(scheme)
{
}

void MountedExternalMemory::LinkMemory(const String& fileName, MemoryBuffer memory)
{
    auto it = files_.find(fileName);
    if (it == files_.end())
        files_.emplace(String{fileName}, memory);
    else
        it->second = memory;
}

void MountedExternalMemory::LinkMemory(const String& fileName, const String& content)
{
    LinkMemory(fileName, MemoryBuffer(content));
}

void MountedExternalMemory::UnlinkMemory(const String& fileName)
{
    auto it = files_.find(fileName);
    if (it != files_.end())
        files_.erase(it);
}

void MountedExternalMemory::SendFileChangedEvent(const String& fileName)
{
    // using namespace FileChanged;

    // VariantMap& eventData = GetEventDataMap();
    // eventData[P_FILENAME] = EMPTY_STRING;
    // eventData[P_RESOURCENAME] = FileIdentifier{scheme_, fileName}.ToUri();
    // SendEvent(E_FILECHANGED, eventData);
    assert(0 && "TODO");
}

bool MountedExternalMemory::AcceptsScheme(const String& scheme) const
{
    return scheme == scheme_;
}

bool MountedExternalMemory::Exists(const FileIdentifier& fileName) const
{
    return AcceptsScheme(fileName.scheme_) && (files_.find(fileName.fileName_) != files_.end());
}

AbstractFilePtr MountedExternalMemory::OpenFile(const FileIdentifier& fileName, FileMode mode)
{
    if (mode & FILE_WRITE)
        return nullptr;

    const auto iter = files_.find(fileName.fileName_);
    if (iter == files_.end())
        return nullptr;

    return std::make_shared<WrappedMemoryBuffer>(iter->second);
}

String MountedExternalMemory::GetName() const
{
    return scheme_;
}

void MountedExternalMemory::Scan(
    std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    if (!flags.Test(SCAN_APPEND))
        result.clear();

    const bool recursive = flags.Test(SCAN_RECURSIVE);
    const String filterExtension = GetExtensionFromFilter(filter);

    for (const auto& [name, _] : files_)
    {
        if (MatchFileName(name, pathName, filterExtension, flags.Test(SCAN_RECURSIVE)))
            result.push_back(TrimPathPrefix(name, pathName));
    }
}

} // namespace Se
