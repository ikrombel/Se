#include "PackageFile.h"

#include <Se/Console.hpp>
#include <Se/IO/FileSystem.h>

namespace Se {

PackageFile::PackageFile() :
    totalSize_(0),
    totalDataSize_(0),
    checksum_(0),
    compressed_(false)
{
}

PackageFile::PackageFile(const String& fileName, unsigned startOffset) :
    totalSize_(0),
    totalDataSize_(0),
    checksum_(0),
    compressed_(false)
{
    Open(fileName, startOffset);
}

PackageFile::~PackageFile() = default;

bool PackageFile::Open(const String& fileName, unsigned startOffset)
{
    AbstractFilePtr file(new File(fileName));
    if (!file->IsOpen())
        return false;

    // Check ID, then read the directory
    file->Seek(startOffset);
    String id = file->ReadFileID();
    if (id != "UPAK" && id != "ULZ4")
    {
        // If start offset has not been explicitly specified, also try to read package size from the end of file
        // to know how much we must rewind to find the package start
        if (!startOffset)
        {
            unsigned fileSize = file->GetSize();
            file->Seek((unsigned)(fileSize - sizeof(unsigned)));
            unsigned newStartOffset = fileSize - file->ReadUInt();
            if (newStartOffset < fileSize)
            {
                startOffset = newStartOffset;
                file->Seek(startOffset);
                id = file->ReadFileID();
            }
        }

        if (id != "UPAK" && id != "ULZ4")
        {
            SE_LOG_ERROR(fileName + " is not a valid package file");
            return false;
        }
    }

    fileName_ = fileName;
    nameHash_ = fileName_;
    totalSize_ = file->GetSize();
    compressed_ = id == "ULZ4";

    unsigned numFiles = file->ReadUInt();
    checksum_ = file->ReadUInt();

    for (unsigned i = 0; i < numFiles; ++i)
    {
        String entryName = file->ReadString();
        PackageEntry newEntry{};
        newEntry.offset_ = file->ReadUInt() + startOffset;
        totalDataSize_ += (newEntry.size_ = file->ReadUInt());
        newEntry.checksum_ = file->ReadUInt();
        if (!compressed_ && newEntry.offset_ + newEntry.size_ > totalSize_)
        {
            SE_LOG_ERROR("File entry {} outside package file", entryName);
            return false;
        }
        else
            entries_[entryName] = newEntry;
    }

    return true;
}

bool PackageFile::Exists(const String& fileName) const
{
    bool found = entries_.find(fileName) != entries_.end();

#ifdef _WIN32
    // On Windows perform a fallback case-insensitive search
    if (!found)
    {
        for (HashMap<String, PackageEntry>::ConstIterator i = entries_.Begin(); i != entries_.End(); ++i)
        {
            if (!i->first_.Compare(fileName, false))
            {
                found = true;
                break;
            }
        }
    }
#endif

    return found;
}

const PackageEntry* PackageFile::GetEntry(const String& fileName) const
{
    auto i = entries_.find(fileName);
    if (i != entries_.end())
        return &i->second;

#ifdef _WIN32
    // On Windows perform a fallback case-insensitive search
    else
    {
        for (auto j = entries_.begin(); j != entries_.end(); ++j)
        {
            if (!j->first.compare(fileName, false))
                return &j->second_;
        }
    }
#endif

    return nullptr;
}


void PackageFile::Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    result.clear();

    String sanitizedPath = GetSanitizedPath(pathName);
    String filterExtension;
    std::size_t dotPos = filter.rfind('.');
    if (dotPos != std::string::npos)
        filterExtension = filter.substr(dotPos);
    if (filterExtension.contains('*'))
        filterExtension.clear();

    bool caseSensitive = true;
#ifdef _WIN32
    // On Windows ignore case in string comparisons
    caseSensitive = false;
#endif

    const std::vector<String>& entryNames = GetEntryNames();
    for (auto i = entryNames.begin(); i != entryNames.end(); ++i)
    {
        String entryName = GetSanitizedPath(*i);
        if ((filterExtension.empty() || entryName.ends_with(filterExtension, caseSensitive)) &&
            entryName.starts_with(sanitizedPath, caseSensitive))
        {
            String fileName = entryName.substr(sanitizedPath.length());
            if (fileName.starts_with("\\") || fileName.starts_with("/"))
                fileName = fileName.substr(1, fileName.length() - 1);
            if (!flags.Test(ScanFlag::SCAN_RECURSIVE) && (fileName.contains("\\") || fileName.contains("/")))
                continue;

            result.push_back(fileName);
        }
    }
}

// /// Checks if mount point accepts scheme.
// bool PackageFile::AcceptsScheme(const String& scheme) const
// {
//     return scheme.empty() || scheme.comparei(GetName()) == 0;
// }

// /// Check if a file exists within the mount point.
// bool PackageFile::Exists(const FileIdentifier& fileName) const
// {
//     // If scheme defined then it should match package name. Otherwise this package can't open the file.
//     if (!AcceptsScheme(fileName.scheme_))
//         return {};

//     // Quit if file doesn't exists in the package.
//     return Exists(fileName.fileName_);
// }

// /// Open package file. Returns null if file not found.
// AbstractFilePtr PackageFile::OpenFile(const FileIdentifier& fileName, FileMode mode)
// {
//     // Package file can write files.
//     if (mode != FILE_READ)
//         return {};

//     // If scheme defined it should match package name. Otherwise this package can't open the file.
//     if (!fileName.scheme_.empty() && fileName.scheme_ != GetName())
//         return {};

//     // Quit if file doesn't exists in the package.
//     if (!Exists(fileName.fileName_))
//         return {};

//     auto file = std::make_shared<File>(this, fileName.fileName_);
//     return file;
// }

bool UnPack(PackageFile* pkg, const String& desteny_dir) {

    SE_LOG_INFO("Unpack to dir: " + desteny_dir);
    for (auto entry : pkg->GetEntries()) {
        auto fs = FileSystem::Get();
        auto filePkgName = entry.first;
        auto fileName = desteny_dir + "/" + filePkgName;

        if (!fs.CreateDirs(desteny_dir, GetPath(filePkgName)))
            continue;

        SE_LOG_INFO("File: " + filePkgName);

        File file(pkg, filePkgName);
        unsigned int fileSize = file.GetSize();
        char* fileData = new char[fileSize];
        file.Read(fileData, fileSize);
        file.Close();

        File newFile(fileName, FileMode::FILE_WRITE);
        newFile.Write(fileData, fileSize);
        newFile.Close();

        delete[] fileData;
    }

    SE_LOG_INFO("Unpack Done");

    return true;
}

// std::optional<FileTime> PackageFile::GetLastModifiedTime(
//     const FileIdentifier& fileName, bool creationIsModification) const
// {
//     if (!Exists(fileName))
//         return std::nullopt;

//     auto fileSystem = GetSubsystem<FileSystem>();
//     return fileSystem->GetLastModifiedTime(fileName_, creationIsModification);
// }

}