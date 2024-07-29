#include "VirtualFileSystem.h"

#include <Se/Console.hpp>
#include <Se/IO/FileSystem.h>

#include <Se/Algorithms.hpp>
#include <SeVFS/FileIdentifier.h>
#include <SeVFS/MountPoint.h>
#include <SeVFS/MountedDirectory.h>
#include <SeVFS/MountedRoot.h>
#include <SeVFS/MountedPackageFile.hpp>
#include <Se/IO/PackageFile.h>

//#include <adaptors.h>

#include <algorithm>
#include <memory>

namespace Se
{

MountPointPtr VirtualFileSystem::MountAliasRoot()
{
    LockGuard lock(mountMutex_);
    return std::dynamic_pointer_cast<MountPoint>(GetOrCreateAliasRoot());
}

std::shared_ptr<MountedAliasRoot> VirtualFileSystem::GetOrCreateAliasRoot()
{
    if (!aliasMountPoint_)
    {
        aliasMountPoint_ = std::make_shared<MountedAliasRoot>();
        mountPoints_.push_back(aliasMountPoint_);
    }

    return aliasMountPoint_;
}

MountPointPtr VirtualFileSystem::MountRoot()
{
    const auto mountPoint = std::make_shared<MountedRoot>();
    Mount(mountPoint);
    return mountPoint;
}

MountPointPtr VirtualFileSystem::MountDir(const String& path)
{
    return MountDir(String::EMPTY, path);
}

MountPointPtr VirtualFileSystem::MountDir(const String& scheme, const String& path)
{
    const auto mountPoint = std::make_shared<MountedDirectory>(path, scheme);
    Mount(mountPoint);
    return mountPoint;
}

void VirtualFileSystem::AutomountDir(const String& path)
{
    AutomountDir(String::EMPTY, path);
}

void VirtualFileSystem::AutomountDir(const String& scheme, const String& path)
{
    auto fileSystem = FileSystem::Get();
    if (!fileSystem.DirExists(path))
        return;

    // Add all the subdirs (non-recursive) as resource directory
    std::vector<String> subdirs;
    fileSystem.ScanDir(subdirs, path, "*", SCAN_DIRS);
    for (const String& dir : subdirs)
    {
        if (dir.starts_with("."))
            continue;

        String autoResourceDir = AddTrailingSlash(path) + dir;
        MountDir(scheme, autoResourceDir);
    }

    // Add all the found package files (non-recursive)
    std::vector<String> packageFiles;
    fileSystem.ScanDir(packageFiles, path, "*.pak", SCAN_FILES);
    for (const String& packageFile : packageFiles)
    {
        if (packageFile.starts_with("."))
            continue;

        String autoPackageName = AddTrailingSlash(path) + packageFile;
        MountPackageFile(autoPackageName);
    }
}

MountPointPtr VirtualFileSystem::MountPackageFile(const String& path)
{
    const auto packageFile = std::make_shared<MountedPackageFile>();
    if (packageFile->Open(path, 0u))
    {
        Mount(packageFile);
        return packageFile;
    }

    return nullptr;
}

void VirtualFileSystem::Mount(MountPointPtr mountPoint)
{
    LockGuard lock(mountMutex_);

    //const MountPointPtr pointPtr{mountPoint};
    if (std::find(mountPoints_.begin(), mountPoints_.end(), mountPoint) != mountPoints_.end())
    {
        return;
    }
    mountPoints_.push_back(mountPoint);

    mountPoint->SetWatching(isWatching_);

    if (auto mountedAliasRoot = std::dynamic_pointer_cast<MountedAliasRoot>(mountPoint))
    {
        if (aliasMountPoint_)
            SE_LOG_WARNING("Mounted alias root when one already exists, overwriting.");
        aliasMountPoint_ = mountedAliasRoot;
    }
}

void VirtualFileSystem::MountAlias(const String& alias, MountPointPtr mountPoint, const String& scheme)
{
    LockGuard lock(mountMutex_);

    GetOrCreateAliasRoot()->AddAlias(alias, scheme, mountPoint);
}

void VirtualFileSystem::MountExistingPackages(
    const std::vector<String>& prefixPaths, const std::vector<String>& relativePaths)
{
    const auto fileSystem = FileSystem::Get();

    for (const String& prefixPath : prefixPaths)
    {
        for (const String& relativePath : relativePaths)
        {
            const String packagePath = prefixPath + relativePath;
            if (fileSystem.FileExists(packagePath))
            {
                MountPointPtr mountPoint = MountPackageFile(packagePath);
                if (mountPoint)
                    MountAlias(format("res:{}", relativePath), mountPoint);
            }
        }
    }
}

void VirtualFileSystem::MountExistingDirectoriesOrPackages(
    const std::vector<String>& prefixPaths, const std::vector<String>& relativePaths)
{
    const auto fileSystem = FileSystem::Get();

    for (const String& prefixPath : prefixPaths)
    {
        for (const String& relativePath : relativePaths)
        {
            const String packagePath = prefixPath + relativePath + ".pak";
            const String directoryPath = prefixPath + relativePath;
            if (fileSystem.FileExists(packagePath))
            {
                MountPointPtr mountPoint = MountPackageFile(packagePath);
                if (mountPoint)
                    MountAlias(format("res:{}", relativePath), mountPoint);
            }
            else if (fileSystem.DirExists(directoryPath))
            {
                MountPointPtr mountPoint = MountDir(directoryPath);
                if (mountPoint)
                    MountAlias(format("res:{}", relativePath), mountPoint);
            }
        }
    }
}

void VirtualFileSystem::Unmount(MountPointPtr mountPoint)
{
    LockGuard lock(mountMutex_);

    if (aliasMountPoint_)
        aliasMountPoint_->RemoveAliases(mountPoint);

    const auto i = std::find(mountPoints_.begin(), mountPoints_.end(), mountPoint);
    if (i != mountPoints_.end())
    {
        // Erase the slow way because order of the mount points matters.
        mountPoints_.erase(i);
    }
}

void VirtualFileSystem::UnmountAll()
{
    LockGuard lock(mountMutex_);

    mountPoints_.clear();
    aliasMountPoint_ = nullptr;
}

MountPointPtr VirtualFileSystem::GetMountPoint(unsigned index) const
{
    LockGuard lock(mountMutex_);

    return (index < mountPoints_.size()) ? mountPoints_[index] : nullptr;
}

AbstractFilePtr VirtualFileSystem::OpenFile(const FileIdentifier& fileName, FileMode mode) const
{
    if (!fileName)
        return nullptr;

    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        if (AbstractFilePtr result = mountPoint->OpenFile(fileName, mode))
            return result;
    }

    return nullptr;
}

String VirtualFileSystem::ReadAllText(const FileIdentifier& fileName) const
{
    AbstractFilePtr file = OpenFile(fileName, FILE_READ);
    if (!file)
        return String::EMPTY;

    const unsigned dataSize = file->GetSize();

    String buffer;
    buffer.resize(dataSize);
    if (file->Read(buffer.data(), dataSize) != dataSize)
        return buffer.substr(0, dataSize);

    return buffer;
}

bool VirtualFileSystem::WriteAllText(const FileIdentifier& fileName, const String& text) const
{
    AbstractFilePtr file = OpenFile(fileName, FILE_WRITE);
    if (!file)
        return false;

    return file->Write(text.data(), text.length()) == text.length();
}

FileTime VirtualFileSystem::GetLastModifiedTime(const FileIdentifier& fileName, bool creationIsModification) const
{
    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        if (const auto result = mountPoint->GetLastModifiedTime(fileName, creationIsModification))
            return *result;
    }

    return 0;
}

String VirtualFileSystem::GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const
{
    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        const String result = mountPoint->GetAbsoluteNameFromIdentifier(fileName);
        if (!result.empty())
            return result;
    }

    return String::EMPTY;
}

FileIdentifier VirtualFileSystem::GetCanonicalIdentifier(const FileIdentifier& fileName) const
{
    FileIdentifier result = fileName;

    // .. is not supported
    result.fileName_.replace("../", "");
    result.fileName_.replace("./", "");
    result.fileName_ = result.fileName_.trimmed();

    // Attempt to go from "file" scheme to local schemes
    if (result.scheme_ == "file")
    {
        if (const auto betterName = GetIdentifierFromAbsoluteName(result.fileName_))
            result = betterName;
    }

    return result;
}

FileIdentifier VirtualFileSystem::GetIdentifierFromAbsoluteName(const String& absoluteFileName) const
{
    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        const FileIdentifier result = mountPoint->GetIdentifierFromAbsoluteName(absoluteFileName);
        if (result)
            return result;
    }

    return FileIdentifier::Empty;
}

FileIdentifier VirtualFileSystem::GetIdentifierFromAbsoluteName(
    const String& scheme, const String& absoluteFileName) const
{
    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        if (!mountPoint->AcceptsScheme(scheme))
            continue;

        const FileIdentifier result = mountPoint->GetIdentifierFromAbsoluteName(absoluteFileName);
        if (result)
            return result;
    }

    return FileIdentifier::Empty;
}

void VirtualFileSystem::SetWatching(bool enable)
{
    if (isWatching_ != enable)
    {
        LockGuard lock(mountMutex_);

        isWatching_ = enable;
        for (auto i = mountPoints_.rbegin(); i != mountPoints_.rend(); ++i)
        {
            (*i)->SetWatching(isWatching_);
        }
    }
}

void VirtualFileSystem::Scan(std::vector<String>& result, const String& scheme, const String& pathName,
    const String& filter, ScanFlags flags) const
{
    LockGuard lock(mountMutex_);

    if (!flags.Test(SCAN_APPEND))
        result.clear();

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        if (mountPoint->AcceptsScheme(scheme))
            mountPoint->Scan(result, pathName, filter, flags | SCAN_APPEND);
    }
}

void VirtualFileSystem::Scan(std::vector<String>& result, const FileIdentifier& pathName, const String& filter,
    ScanFlags flags) const
{
    Scan(result, pathName.scheme_, pathName.fileName_, filter, flags);
}

bool VirtualFileSystem::Exists(const FileIdentifier& fileName) const
{
    LockGuard lock(mountMutex_);

    for (MountPointPtr mountPoint : Reverse(mountPoints_))
    {
        if (mountPoint->Exists(fileName))
            return true;
    }
    return false;
}

VirtualFileSystem* VirtualFileSystem::Get()
{
    static VirtualFileSystem* ptr_;
    if (!ptr_) {
        SE_LOG_INFO("VirtualFileSystem initialized.");
        ptr_ = new VirtualFileSystem();
    }
    return ptr_;
}


MountPointGuard::MountPointGuard(MountPointPtr mountPoint)
    : mountPoint_(mountPoint)
{
    if (mountPoint_)
    {
        VirtualFileSystem::Get()->Mount(mountPoint_);
    }
}

MountPointGuard::~MountPointGuard()
{
    Release();
}

MountPointGuard::MountPointGuard(MountPointGuard&& other) noexcept
{
    *this = std::move(other);
}

MountPointGuard& MountPointGuard::operator=(MountPointGuard&& other) noexcept
{
    Release();
    mountPoint_ = std::move(other.mountPoint_);
    return *this;
}

void MountPointGuard::Release()
{
    if (mountPoint_)
    {
        VirtualFileSystem::Get()->Unmount(mountPoint_);
        mountPoint_ = nullptr;
    }
}



} // namespace Se
