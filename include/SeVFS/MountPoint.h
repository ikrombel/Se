#pragma once

#include <Se/IO/AbstractFile.hpp>
#include <Se/IO/ScanFlags.hpp>
#include <SeVFS/FileIdentifier.h>

#include <Se/IO/File.h>

#include <optional>
#include <memory>

namespace Se
{

/// Access to engine file system mount point.
class MountPoint
{
public:
    /// Construct.
    explicit MountPoint() {};
    /// Destruct.
    virtual ~MountPoint() = default;

    /// Checks if mount point accepts scheme.
    virtual bool AcceptsScheme(const String& scheme) const = 0;

    /// Check if a file exists within the mount point.
    /// The file name may be be case-insensitive on Windows and case-sensitive on other platforms.
    virtual bool Exists(const FileIdentifier& fileName) const = 0;

    /// Open file in a virtual file system. Returns null if file not found.
    /// The file name may be be case-insensitive on Windows and case-sensitive on other platforms.
    virtual AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) = 0;

    /// Return modification time, or 0 if not supported.
    /// Return nullopt if file does not exist.
    virtual std::optional<FileTime> GetLastModifiedTime(const FileIdentifier& fileName, bool creationIsModification) const
    {
        if (Exists(fileName))
            return 0u;
        else
            return std::nullopt;
    }

    /// Returns human-readable name of the mount point.
    virtual String GetName() const
    {
        return String::EMPTY;
    }

    /// Return absolute file name for *existing* identifier in this mount point, if supported.
    virtual String GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const {
        return String::EMPTY; }

    /// Return identifier in this mount point for absolute file name, if supported.
    /// Works even if the file does not exist.
    virtual FileIdentifier GetIdentifierFromAbsoluteName(const String& absoluteFileName) const;

    /// Enable or disable FileChanged events.
    virtual void SetWatching(bool enable);

    /// Returns true if FileChanged events are enabled.
    virtual bool IsWatching() const;

    /// Enumerate objects in the mount point. Only files enumeration is guaranteed to be supported.
    virtual void Scan(std::vector<String>& result, const String& pathName, const String& filter,
        ScanFlags flags) const = 0;
};

/// Base implementation of watchable mount point.
class WatchableMountPoint : public MountPoint
{
    
public:
    explicit WatchableMountPoint();
    ~WatchableMountPoint() override;

    /// Implement MountPoint.
    /// @{
    void SetWatching(bool enable) override;
    bool IsWatching() const override;
    /// @}

protected:
    /// Start file watcher.
    virtual void StartWatching() = 0;
    /// Stop file watcher.
    virtual void StopWatching() = 0;

private:
    bool isWatching_{};
};

using MountPointPtr = std::shared_ptr<MountPoint>;

} // namespace Se
