#pragma once

#include <SeVFS/MountPoint.h>
#include <Se/IO/PackageFile.h>


namespace Se {

/// Include PackageFile to virtual file system
class MountedPackageFile : public PackageFile, public MountPoint
{
public:
    MountedPackageFile() : 
            PackageFile(),
            MountPoint() {};

    /// Construct and open.
    MountedPackageFile(const String& fileName, unsigned startOffset = 0) :
            MountPoint(),
            PackageFile(fileName, startOffset)
    {
        
    }

    /// Implement MountPoint.
    /// @{
    bool AcceptsScheme(const String& scheme) const override;
    bool Exists(const FileIdentifier& fileName) const override;
    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) override;
    std::optional<FileTime> GetLastModifiedTime(
        const FileIdentifier& fileName, bool creationIsModification) const override;

    String GetName() const override { return fileName_; }

    void Scan(std::vector<String>& result, const String& pathName, const String& filter,
        ScanFlags flags) const override {
            PackageFile::Scan(result, pathName, filter, flags);
        }

    /// @}


};

/// Checks if mount point accepts scheme.
inline bool MountedPackageFile::AcceptsScheme(const String& scheme) const
{
    return scheme.empty() || scheme.comparei(GetName()) == 0;
}

/// Check if a file exists within the mount point.
inline bool MountedPackageFile::Exists(const FileIdentifier& fileName) const
{
    // If scheme defined then it should match package name. Otherwise this package can't open the file.
    if (!AcceptsScheme(fileName.scheme_))
        return false;

    // Quit if file doesn't exists in the package.
    return PackageFile::Exists(fileName.fileName_);
}

/// Open package file. Returns null if file not found.
inline AbstractFilePtr MountedPackageFile::OpenFile(const FileIdentifier& fileName, FileMode mode)
{
    // Package file can write files.
    if (mode != FILE_READ)
        return {};

    // If scheme defined it should match package name. Otherwise this package can't open the file.
    if (!fileName.scheme_.empty() && fileName.scheme_ != GetName())
        return {};

    // Quit if file doesn't exists in the package.
    if (!Exists(fileName.fileName_))
        return {};

    auto file = std::make_shared<File>(this, fileName.fileName_);
    file->SetName(fileName.ToUri());
    return file;
}

inline std::optional<FileTime> MountedPackageFile::GetLastModifiedTime(
    const FileIdentifier& fileName, bool creationIsModification) const
{
    if (!Exists(fileName))
        return std::nullopt;

    return FileSystem::Get().GetLastModifiedTime(fileName_, creationIsModification);
}

}