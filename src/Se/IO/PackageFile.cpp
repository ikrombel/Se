#include "PackageFile.h"

#include <Se/Console.hpp>
#include <Se/IO/FileSystem.h>

#include <algorithm>

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
    AbstractFilePtr file = std::make_shared<File>(fileName);
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
        for (auto i = entries_.begin(); i != entries_.end(); ++i)
        {
            if (!i->first.comparei(fileName))
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
            if (!j->first.comparei(fileName))
                return &j->second;
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


void PackageFile::ScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    result.Children.clear();

    auto entries = GetEntries();
    for (auto entry : entries) {
        TreeNodeAddPath(&result, entry.first);
    }
}


}