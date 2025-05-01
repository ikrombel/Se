#include "PackageFile.h"

#include "Console.hpp"

#ifdef HAVE_LZ4
#include <LZ4/lz4.h>
#include <LZ4/lz4hc.h>
#endif


namespace Se {



struct FileEntry
{
    String name_;
    unsigned offset_{};
    unsigned size_{};
    unsigned checksum_{};
};


static const unsigned COMPRESSED_BLOCK_SIZE = 32768;
unsigned blockSize_ = COMPRESSED_BLOCK_SIZE;

// String SimplifyPath(String path) {
//     std::vector<String> folders;
//     int idx = 0;
//     while (idx < path.length()) {
//         while (idx < path.length() && path.at(idx) == '/')
//             idx++;
//         String f = "";
//         while (idx < path.length() && path.at(idx) != '/') {
//             f += path.at(idx);
//             idx++;
//         }
//         if (f == ".") {
//             continue;
//         } else if (f == "..") {
//             if (!folders.empty()) {
//                 folders.pop_back();
//             }
//         } else if (f.length()) {
//             folders.push_back(f);
//         }
//     }

//     String result = "";
//     int i = 0;
//     for (auto fo : folders) {
// #ifdef WIN32
//         if (i == 0) {
//             i = 1;
//         }
//         else
// #endif
//             result += "/";
//         result += fo;
//     }
//     if (result.empty()) {
//         result = "/";
//     }
//     return result;
// }


String ignoreExtensions_[] = {
    ".bak",
    ".rule",
    ""
};

bool CheckChanges(const String& rootDir, std::vector<FileEntry> entries, String packageName) {

    unsigned totalDataSize{};

    unsigned checksumEntries{};
    unsigned checksumPackage{};

    auto fs = FileSystem::Get();

    String outputPath = GetPath(packageName);
    if (!fs.DirExists(outputPath)) {
        fs.CreateDir(outputPath);
    }

    PackageFile pkgFile(packageName);
    checksumPackage = pkgFile.GetChecksum();

    // Write file data, calculate checksums & correct offsets
    for (unsigned i = 0; i < entries.size(); ++i)
    {
        String fileFullPath = rootDir + "/" + entries[i].name_;

        File srcFile(fileFullPath);
        if (!srcFile.IsOpen()) {
            SE_LOG_ERROR("Could not open file " + fileFullPath);
            return false;
        }

        unsigned dataSize = entries[i].size_;
        totalDataSize += dataSize;
        std::shared_ptr<unsigned char> buffer(new unsigned char[dataSize], std::default_delete<unsigned char[]>());

        if (srcFile.Read(buffer.get(), dataSize) != dataSize) {
            SE_LOG_ERROR("Could not read file " + fileFullPath);
            return false;
        }
        srcFile.Close();

        for (unsigned j = 0; j < dataSize; ++j)
        {
            checksumEntries = SDBMHash(checksumEntries, buffer.get()[j]);
            entries[i].checksum_ = SDBMHash(entries[i].checksum_, buffer.get()[j]);
        }

    }
    bool changed = (checksumEntries != checksumPackage);
    if (changed)
        SE_LOG_INFO("Package updated: {}", packageName);
    return changed;
}

class PackageTool {
public:
    PackageTool() {}
    virtual ~PackageTool() = default;

    bool Pack(const String& inputDir, const String& packageName, bool compress);

    bool Pack(const String& packPath, const std::unordered_map<String, AbstractFilePtr>& files, bool compress = false)
    {
        return WritePackageFile(packPath, files, compress);
    }

    void Unpack(const String& packageName, const String& dirName);

private:
    void WritePackageFile(const String &fileName, const String &rootDir);
    void ProcessFile(const String& fileName, const String& rootDir);
    void WriteHeader(File& dest);

    bool WritePackageFile(const String& fileName, std::unordered_map<String, AbstractFilePtr> files, bool compress = false);

    String basePath_;
    String pkgName_;
    String rootDir_;
    bool compress_;
    std::vector<FileEntry> entries_;

    unsigned checksum_ = 0;
};

bool PackageTool::Pack(const String& inputDir, const String& packageName, bool compress)
{

    basePath_ = AddTrailingSlash(FileSystem::SimplifyPath(inputDir));
    pkgName_ = packageName;
    compress_ = compress;
    checksum_ = 0;
    entries_.clear();

    auto fileSystem = FileSystem::Get();
    SE_LOG_INFO("Scanning directory " + basePath_ + " for files");

    if (!fileSystem.DirExists(inputDir)) {
        SE_LOG_ERROR("Input dir is uncorrect: ", inputDir);
        return false;
    }

    // Get the file list recursively
    std::vector<String> fileNames;
    fileSystem.ScanDir(fileNames, basePath_, "", SCAN_FILES | SCAN_RECURSIVE);
    if (!fileNames.size()) {
        SE_LOG_ERROR("No files found");
        return false;
    }

    // Check for extensions to ignore
    for (unsigned i = fileNames.size() - 1; i < fileNames.size(); --i)
    {
        String extension = GetExtension(fileNames[i]);
        for (unsigned j = 0; j < ignoreExtensions_[j].length(); ++j)
        {
            if (extension == ignoreExtensions_[j])
            {
                fileNames.erase(fileNames.begin() + i);
                break;
            }
        }
    }

    for (unsigned i = 0; i < fileNames.size(); ++i)
        ProcessFile(fileNames[i], basePath_);

    
    if (!CheckChanges(inputDir, entries_, packageName))
        return false;
    WritePackageFile(pkgName_, basePath_);
    return true;
}

//--------------------------------------------------------------------------------------------


void PackageTool::ProcessFile(const String& fileName, const String& rootDir)
{
    String fullPath = rootDir + "/" + fileName;
    File file;
    if (!file.Open(fullPath))
        SE_LOG_ERROR("Could not open file " + fileName);
    if (!file.GetSize())
        return;

    FileEntry newEntry;
    newEntry.name_ = fileName;
    newEntry.offset_ = 0; // Offset not yet known
    newEntry.size_ = file.GetSize();
    newEntry.checksum_ = 0; // Will be calculated later
    entries_.push_back(newEntry);
}

bool PackageTool::WritePackageFile(const String& fileName, std::unordered_map<String, AbstractFilePtr> files, bool compress)
{
    File dest;
    if (!dest.Open(fileName, FILE_WRITE)) {
        SE_LOG_ERROR("Could not open output file " + fileName);
        return false;
    }

    entries_.clear();

    for (auto it : files)
    {
        FileEntry newEntry;
        newEntry.name_ = it.first;
        newEntry.offset_ = 0; // Offset not yet known
        newEntry.size_ = it.second->GetSize();
        newEntry.checksum_ = 0; // Will be calculated later
        entries_.push_back(newEntry);
    }

    String checksumFileData;

    for (auto it : files)
    {
        unsigned checksum = it.second->GetChecksum();
        checksumFileData += format("{}:{}\n", it.first, checksum);
    }

    // Write ID, number of files & placeholder for checksum
    WriteHeader(dest);

    auto writeEntry = [&](const FileEntry& entry) {
        dest.WriteString(entry.name_);
        dest.WriteUInt(entry.offset_);
        dest.WriteUInt(entry.size_);
        dest.WriteUInt(entry.checksum_);
    };

    for (unsigned i = 0; i < entries_.size(); ++i)
    {
        // Write entry (correct offset is still unknown, will be filled in later)
        writeEntry(entries_[i]);
    }

    unsigned totalDataSize = 0;
    unsigned lastOffset;

    // Write file data, calculate checksums & correct offsets
    for (unsigned i = 0; i < entries_.size(); ++i)
    {
        lastOffset = entries_[i].offset_ = dest.GetSize();
        String fileFullPath = entries_[i].name_;

        auto& srcFile = files[fileFullPath];
        if (!srcFile->GetSize())
            SE_LOG_ERROR("Could not open file " + fileFullPath);

        unsigned dataSize = entries_[i].size_;
        totalDataSize += dataSize;
        std::shared_ptr<unsigned char> buffer(new unsigned char[dataSize], std::default_delete<unsigned char[]>());

        if (srcFile->Read(buffer.get(), dataSize) != dataSize)
            SE_LOG_ERROR("Could not read file " + fileFullPath);
        srcFile->Close();

        for (unsigned j = 0; j < dataSize; ++j)
        {
            checksum_ = SDBMHash(checksum_, buffer.get()[j]);
            entries_[i].checksum_ = SDBMHash(entries_[i].checksum_, buffer.get()[j]);
        }

        if (!compress)
        {
            dest.Write(buffer.get(), entries_[i].size_);
        }
        else
        {
            std::shared_ptr<unsigned char> compressBuffer(new unsigned char[LZ4_compressBound(blockSize_)], std::default_delete<unsigned char[]>());

            unsigned pos = 0;

            while (pos < dataSize)
            {
                unsigned unpackedSize = blockSize_;
                if (pos + unpackedSize > dataSize)
                    unpackedSize = dataSize - pos;

                auto packedSize = (unsigned)LZ4_compress_HC((const char*)&(buffer.get()[pos]), 
                        (char*)compressBuffer.get(), unpackedSize, LZ4_compressBound(unpackedSize), 0);
                if (!packedSize)
                    SE_LOG_ERROR("LZ4 compression failed for file {} at offset {}", entries_[i].name_, pos);

                dest.WriteUShort((unsigned short)unpackedSize);
                dest.WriteUShort((unsigned short)packedSize);
                dest.Write(compressBuffer.get(), packedSize);

                pos += unpackedSize;
            }
        }
        unsigned totalPackedBytes = dest.GetSize() - lastOffset;

        SE_LOG_INFO("Total packed bytes for {}: {}", entries_[i].name_, totalPackedBytes);
    }
    

    SE_LOG_INFO("Total data size for package {}: {}", fileName, totalDataSize);
    return true;
}

void PackageTool::WritePackageFile(const String& fileName, const String& rootDir)
{
    SE_LOG_INFO("Writing package");

    File dest;
    if (!dest.Open(fileName, FILE_WRITE))
        SE_LOG_ERROR("Could not open output file " + fileName);

    // Write ID, number of files & placeholder for checksum
    WriteHeader(dest);

    for (unsigned i = 0; i < entries_.size(); ++i)
    {
        // Write entry (correct offset is still unknown, will be filled in later)
        dest.WriteString(entries_[i].name_);
        dest.WriteUInt(entries_[i].offset_);
        dest.WriteUInt(entries_[i].size_);
        dest.WriteUInt(entries_[i].checksum_);
    }

    unsigned totalDataSize = 0;
    unsigned lastOffset;

    // Write file data, calculate checksums & correct offsets
    for (unsigned i = 0; i < entries_.size(); ++i)
    {
        lastOffset = entries_[i].offset_ = dest.GetSize();
        String fileFullPath = rootDir + "/" + entries_[i].name_;

        File srcFile(fileFullPath);
        if (!srcFile.IsOpen())
            SE_LOG_ERROR("Could not open file " + fileFullPath);

        unsigned dataSize = entries_[i].size_;
        totalDataSize += dataSize;
        std::shared_ptr<unsigned char> buffer(new unsigned char[dataSize], std::default_delete<unsigned char[]>());

        if (srcFile.Read(buffer.get(), dataSize) != dataSize)
            SE_LOG_ERROR("Could not read file " + fileFullPath);
        srcFile.Close();

        for (unsigned j = 0; j < dataSize; ++j)
        {
            checksum_ = SDBMHash(checksum_, buffer.get()[j]);
            entries_[i].checksum_ = SDBMHash(entries_[i].checksum_, buffer.get()[j]);
        }

        if (!compress_)
        {
//            if (!quiet_)
//                PrintLine(entries_[i].name_ + " size " + String(dataSize));
            dest.Write(buffer.get(), entries_[i].size_);
        }
        else
        {
            std::shared_ptr<unsigned char> compressBuffer(new unsigned char[LZ4_compressBound(blockSize_)], std::default_delete<unsigned char[]>());

            unsigned pos = 0;

            while (pos < dataSize)
            {
                unsigned unpackedSize = blockSize_;
                if (pos + unpackedSize > dataSize)
                    unpackedSize = dataSize - pos;

                auto packedSize = (unsigned)LZ4_compress_HC((const char*)&(buffer.get()[pos]), (char*)compressBuffer.get(), unpackedSize, LZ4_compressBound(unpackedSize), 0);
                if (!packedSize)
                    SE_LOG_ERROR("LZ4 compression failed for file {} at offset {}", entries_[i].name_, pos);

                dest.WriteUShort((unsigned short)unpackedSize);
                dest.WriteUShort((unsigned short)packedSize);
                dest.Write(compressBuffer.get(), packedSize);

                pos += unpackedSize;
            }

//            if (!quiet_)
//            {
                unsigned totalPackedBytes = dest.GetSize() - lastOffset;
                String fileEntry(entries_[i].name_);
                fileEntry += cformat("\tin: %u\tout: %u\tratio: %f", dataSize, totalPackedBytes,
                    totalPackedBytes ? 1.f * dataSize / totalPackedBytes : 0.f);
                SE_LOG_INFO(fileEntry);
//            }
        }
    }

    // Write package size to the end of file to allow finding it linked to an executable file
    unsigned currentSize = dest.GetSize();
    dest.WriteUInt(currentSize + sizeof(unsigned));

    // Write header again with correct offsets & checksums
    dest.Seek(0);
    WriteHeader(dest);

    for (unsigned i = 0; i < entries_.size(); ++i)
    {
        dest.WriteString(entries_[i].name_);
        dest.WriteUInt(entries_[i].offset_);
        dest.WriteUInt(entries_[i].size_);
        dest.WriteUInt(entries_[i].checksum_);
    }
    SE_LOG_INFO(
        "Package:"
        "\nNumber of files: {}"
        "\nFile data size: {}"
        "\nPackage size: {}"
        "\nChecksum: {}"
        "\nCompressed: {}",
        entries_.size(), totalDataSize, dest.GetSize(), checksum_, compress_ ? "yes" : "no");

}

void PackageTool::WriteHeader(File& dest)
{
    if (!compress_)
        dest.WriteFileID("UPAK");
    else
        dest.WriteFileID("ULZ4");
    dest.WriteUInt(entries_.size());
    dest.WriteUInt(checksum_);
}

void PackageTool::Unpack(const String& packageName, const String& dirName)
{
    auto packageFile = std::make_shared<PackageFile>(packageName);
    auto fs = FileSystem::Get();

    char buffer[1024];

    const std::unordered_map<String, PackageEntry>& entries = packageFile->GetEntries();
    for (auto i = entries.begin(); i != entries.end();)
    {
        std::unordered_map<String, PackageEntry>::const_iterator current = i++;
        String outFilePath(dirName + "/" + current->first);
        int32_t pos = outFilePath.find_last('/');
        if (pos == String::npos)
            SE_LOG_ERROR("pos == String::npos");

        fs.CreateDir(outFilePath.substr(0, pos));

        File packedFile(packageFile.get(), current->first);
        if (!packedFile.IsOpen())
            SE_LOG_ERROR("packedFile open failed " + current->first);

        File outFile(outFilePath, FILE_WRITE);
        if (!outFile.IsOpen())
            SE_LOG_ERROR("outFile open failed " + current->first);

        printf("Write file: %s", outFilePath.c_str());

        unsigned numRead, numWrite;

        while (true)
        {
            numRead = packedFile.Read(buffer, sizeof(buffer));
            if (!numRead)
            {
                packedFile.Close();
                outFile.Close();
                break;
            }

            numWrite = outFile.Write(buffer, numRead);
            if (numWrite != numRead)
                SE_LOG_ERROR("numWrite != numRead");
        }
    }

    printf("Done");
}

namespace Tool {

bool Pack(const String& inputDir, const String& packageName, bool compress)
{
    return PackageTool().Pack(inputDir, packageName, compress);
}


void Unpack(const String& packageName, const String& dirName)
{
    PackageTool().Unpack(packageName, dirName);
}

} //namespace Tool

} // namespace Se