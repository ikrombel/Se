#pragma once

#include <SeVFS/MountPoint.h>

#include <tuple>
#include <unordered_map>

namespace Se
{

/// Mount point that provides named aliases to other mount points.
class MountedAliasRoot : public MountPoint
{
public:
    explicit MountedAliasRoot();
    ~MountedAliasRoot() override;

    /// Add alias to another mount point.
    void AddAlias(const String& path, const String& scheme, std::shared_ptr<MountPoint> mountPoint);
    /// Remove all aliases to the mount point.
    void RemoveAliases(MountPointPtr mountPoint);
    /// Find mount point and its alias for the specified file name.
    /// Returns mount point, alias and recommended scheme.
    std::tuple<MountPointPtr, String, String> FindMountPoint(String fileName) const;

    /// Implement MountPoint.
    /// @{
    bool AcceptsScheme(const String& scheme) const override;

    bool Exists(const FileIdentifier& fileName) const override;

    AbstractFilePtr OpenFile(const FileIdentifier& fileName, FileMode mode) override;

    std::optional<FileTime> GetLastModifiedTime(
        const FileIdentifier& fileName, bool creationIsModification) const override;

    String GetName() const override;

    String GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const override;

    FileIdentifier GetIdentifierFromAbsoluteName(const String& absoluteFileName) const override;

    void Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const override;
    /// @}

private:
    std::unordered_map<String, std::pair<std::shared_ptr<MountPoint>, String>> aliases_;
};

} // namespace
