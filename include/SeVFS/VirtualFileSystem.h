#pragma once

#include <Se/String.hpp>
#include <Se/NonCopyable.hpp>
#include <Se/IO/AbstractFile.hpp>
#include <SeVFS/MountPoint.h>
#include <SeVFS/MountedAliasRoot.h>

#include <Se/Mutex.hpp>

#include <memory>
#include <vector>
#include <map>

namespace Se
{

/// Subsystem for virtual file system.
class VirtualFileSystem
{
public:
    /// Construct.
    explicit VirtualFileSystem() = default;
    /// Destruct.
    virtual ~VirtualFileSystem() = default;

    /// Mount alias root as alias:// scheme. Alias root will be mounted automatically if alias is created.
    MountPointPtr MountAliasRoot();
    /// Mount file system root as file:// scheme.
    MountPointPtr MountRoot();
    /// Mount real folder into virtual file system.
    MountPointPtr MountDir(const String& path);
    /// Mount real folder into virtual file system under the scheme.
    MountPointPtr MountDir(const String& scheme, const String& path);
    /// Mount subfolders and pak files from real folder into virtual file system.
    void AutomountDir(const String& path);
    /// Mount subfolders and pak files from real folder into virtual file system under the scheme.
    void AutomountDir(const String& scheme, const String& path);
    /// Mount package file into virtual file system.
    MountPointPtr MountPackageFile(const String& path);
    /// Mount virtual or real folder into virtual file system.
    void Mount(MountPointPtr mountPoint);
    /// Mount alias to another mount point.
    void MountAlias(const String& alias, MountPointPtr mountPoint, const String& scheme = String::EMPTY);
    /// Remove mount point from the virtual file system.
    void Unmount(MountPointPtr mountPoint);
    /// Remove all mount points.
    void UnmountAll();
    /// Get number of mount points.
    unsigned NumMountPoints() const { return mountPoints_.size(); }
    /// Get mount point by index.
    MountPointPtr GetMountPoint(unsigned index) const;

    /// Mount all existing packages for each combination of prefix path and relative path.
    void MountExistingPackages(const std::vector<String>& prefixPaths, const std::vector<String>& relativePaths);
    /// Mount all existing directories and packages for each combination of prefix path and relative path.
    /// Package is preferred over directory if both exist.
    void MountExistingDirectoriesOrPackages(const std::vector<String>& prefixPaths, const std::vector<String>& relativePaths);

    /// Check if a file exists in the virtual file system.
    bool Exists(const FileIdentifier& fileName) const;
    /// Open file in the virtual file system. Returns null if file not found.
    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) const;
    /// Read text file from the virtual file system. Returns empty string if file not found.
    String ReadAllText(const FileIdentifier& fileName) const;
    /// Write text file to the virtual file system. Returns true if file is written successfully.
    bool WriteAllText(const FileIdentifier& fileName, const String& text) const;
    /// Return modification time. Return 0 if not supported or file doesn't exist.
    FileTime GetLastModifiedTime(const FileIdentifier& fileName, bool creationIsModification) const;
    /// Return absolute file name for *existing* identifier in this mount point, if supported.
    String GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const;
    /// Return canonical file identifier, if possible.
    FileIdentifier GetCanonicalIdentifier(const FileIdentifier& fileName) const;
    /// Works even if the file does not exist.
    FileIdentifier GetIdentifierFromAbsoluteName(const String& absoluteFileName) const;
    /// Return relative file name of the file, or empty if not found.
    FileIdentifier GetIdentifierFromAbsoluteName(const String& scheme, const String& absoluteFileName) const;

    /// Enable or disable file watchers.
    void SetWatching(bool enable);

    /// Returns true if the file watchers are enabled.
    bool IsWatching() const { return isWatching_; }

    /// Scan for specified files.
    void Scan(std::vector<String>& result, const String& scheme, const String& pathName,
        const String& filter, ScanFlags flags) const;
    void Scan(std::vector<String>& result, const FileIdentifier& pathName, const String& filter,
        ScanFlags flags) const;

    static VirtualFileSystem* Get();

private:
    /// Return or create internal alias:// mount point.
    std::shared_ptr<MountedAliasRoot> GetOrCreateAliasRoot();

    /// Mutex for thread-safe access to the mount points.
    mutable Mutex mountMutex_;
    /// File system mount points. It is expected to have small number of mount points.
    std::vector<MountPointPtr> mountPoints_;
    /// Alias mount point.
    std::shared_ptr<MountedAliasRoot> aliasMountPoint_;
    /// Are file watchers enabled.
    bool isWatching_{};
};

/// Helper class to mount and unmount an object automatically.
class MountPointGuard : public MovableNonCopyable
{
public:
    explicit MountPointGuard(MountPointPtr mountPoint);
    ~MountPointGuard();

    MountPointGuard(MountPointGuard&& other) noexcept;
    MountPointGuard& operator=(MountPointGuard&& other) noexcept;

    void Release();
    MountPointPtr Get() const { return mountPoint_; }

    template <class T>
    explicit MountPointGuard(const std::shared_ptr<T>& mountPoint)
        : MountPointGuard(mountPoint.get())
    {
    }

private:
    MountPointPtr mountPoint_;
};

} // namespace Se
