// Copyright (c) 2023-2023 the rbfx project.


#pragma once

#include <Se/IO/AbstractFile.hpp>
#include <SeVFS/MountPoint.h>
#include <Se/IO/MemoryBuffer.hpp>

#include <unordered_map>

namespace Se
{

/// Lightweight mount point that provides read-only access to the externally managed memory.
class MountedExternalMemory : public MountPoint
{
public:
    explicit MountedExternalMemory(const String& scheme);

    void LinkMemory(const String& fileName, MemoryBuffer memory);
    void LinkMemory(const String& fileName, const String& content);
    void UnlinkMemory(const String& fileName);

    void SendFileChangedEvent(const String& fileName);

    /// Implement MountPoint.
    /// @{
    bool AcceptsScheme(const String& scheme) const override;
    bool Exists(const FileIdentifier& fileName) const override;
    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) override;

    String GetName() const override;

    void Scan(std::vector<String>& result, const String& pathName, const String& filter,
        ScanFlags flags) const override;
    /// @}

private:
    String scheme_;
    std::unordered_map<String, MemoryBuffer> files_{};
};

} // namespace Se
