// Copyright (c) 2022-2023 the Urho3D project.

#pragma once

#include <Se/String.hpp>
#include <Se/IO/File.h>
#include <SeVFS/FileWatcher.h>
#include <SeVFS/MountPoint.h>

namespace Se
{

/// Stores files of a directory tree sequentially for convenient access.
class MountedDirectory : public WatchableMountPoint
{

public:
    /// Construct and open.
    MountedDirectory(const String& directory, String scheme = String::EMPTY);
    /// Destruct.
    ~MountedDirectory() override;

    /// Implement MountPoint.
    /// @{
    bool AcceptsScheme(const String& scheme) const override;
    bool Exists(const FileIdentifier& fileName) const override;
    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) override;
    std::optional<FileTime> GetLastModifiedTime(
        const FileIdentifier& fileName, bool creationIsModification) const override;

    String GetName() const override { return name_; }

    String GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const override;
    FileIdentifier GetIdentifierFromAbsoluteName(const String& absoluteFileName) const override;

    void Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const override;
    /// @}

    /// Get mounted directory path.
    const String& GetDirectory() const { return directory_; }

protected:
    String SanitizeDirName(const String& name) const;

    /// Implement WatchableMountPoint.
    /// @{
    void StartWatching() override;
    void StopWatching() override;
    /// @}

private:
    void ProcessUpdates();

private:
    /// Expected file locator scheme.
    const String scheme_;
    /// Target directory.
    const String directory_;
    /// Name of the mount point.
    const String name_;
    /// File watcher for resource directory, if automatic reloading enabled.
    std::shared_ptr<FileWatcher> fileWatcher_;
};

} // namespace Se
