// Copyright (c) 2008-2019 the GFrost project.

#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <Se/String.hpp>
#include <Se/StringHash.hpp>
#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>
#include <Se/IO/MemoryBuffer.hpp>

namespace Se
{

/// %File entry within the package file.
struct PackageEntry
{
    /// Offset from the beginning.
    unsigned offset_;
    /// File size.
    unsigned size_;
    /// File checksum.
    unsigned checksum_;
};

/// Stores files of a directory tree sequentially for convenient access.
class PackageFile
{
public:
    /// Construct.
    explicit PackageFile();
    /// Construct and open.
    PackageFile(const String& fileName, unsigned startOffset = 0);
    /// Destruct.
    virtual ~PackageFile();

    /// Open the package file. Return true if successful.
    bool Open(const String& fileName, unsigned startOffset = 0);
    /// Check if a file exists within the package file. This will be case-insensitive on Windows and case-sensitive on other platforms.
    bool Exists(const String& fileName) const;
    /// Return the file entry corresponding to the name, or null if not found. This will be case-insensitive on Windows and case-sensitive on other platforms.
    const PackageEntry* GetEntry(const String& fileName) const;

    /// Return all file entries.
    const std::unordered_map<String, PackageEntry>& GetEntries() const { 
        return entries_; }

    /// Return the package file name.
    const String& GetName() const { return fileName_; }

    /// Return hash of the package file name.
    StringHash GetNameHash() const { return nameHash_; }

    /// Return number of files.
    unsigned GetNumFiles() const { return entries_.size(); }

    /// Return total size of the package file.
    unsigned GetTotalSize() const { return totalSize_; }

    /// Return total data size from all the file entries in the package file.
    unsigned GetTotalDataSize() const { return totalDataSize_; }

    /// Return checksum of the package file contents.
    unsigned GetChecksum() const { return checksum_; }

    /// Return whether the files are compressed.
    bool IsCompressed() const { return compressed_; }

    /// Return list of file names in the package.
    const std::vector<String> GetEntryNames() const { 
        std::vector<String> keys;
        keys.reserve(entries_.size());
        for(auto it : entries_) {
            keys.push_back(it.first);
        }
        return keys;
        //return entries_.keys();
    }

    /// Scan package for specified files.
    void Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const;

    void ScanTree(DirectoryNode& result, const String& path,
                    const String& filter, ScanFlags flags) const;

private:
    /// File entries.
    std::unordered_map<String, PackageEntry> entries_;
    /// Package file name hash.
    StringHash nameHash_;
    /// Package file total size.
    unsigned totalSize_;
    /// Total data size in the package using each entry's actual size if it is a compressed package file.
    unsigned totalDataSize_;
    /// Package file checksum.
    unsigned checksum_;
    /// Compressed flag.
    bool compressed_;
protected:
    /// File name.
    String fileName_;
};




//declarate in file PackageFile.Tool.cpp

namespace Tool {

bool Pack(const String& inputDir, const String& packageName, bool compress = true);

void Unpack(const String& packageName, const String& dirName);

} // namespace Tool

} // namespace Se