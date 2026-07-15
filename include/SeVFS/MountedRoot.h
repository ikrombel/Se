// Copyright (c) 2023-2023 the rbfx project.

#pragma once

#include <Se/IO/AbstractFile.hpp>
#include <SeVFS/MountPoint.h>
#include <Se/IO/MemoryBuffer.hpp>

namespace Se
{

/// Lightweight mount point that provides read-only access to the externally managed memory.
class MountedRoot : public MountPoint
{
public:
    explicit MountedRoot();

    /// Implement MountPoint.
    /// @{
    bool AcceptsScheme(const String& scheme) const override;

    bool Exists(const FileIdentifier& fileName) const override;

    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) override;

    String GetName() const override;

    String GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const override;

    FileIdentifier GetIdentifierFromAbsoluteName(const String& absoluteFileName) const override;

    void Scan(std::vector<String>& result, const String& pathName, const String& filter,
        ScanFlags flags) const override;
    /// @}
};

} // namespace Se
