// Copyright (c) 2008-2019 the GFrost project.

#pragma once


#include <Se/IO/AbstractFile.hpp>
#include <Se/IO/MemoryBuffer.hpp>
#include <Se/Math.hpp>

#include <cassert>
#include <limits>

#ifdef __ANDROID__
#include <Se/Platform/AndroidFile.hpp>
#endif


// #ifdef __ANDROID__
// struct SDL_RWops;
// #endif

namespace Se
{

class PackageFile;

// #ifdef __ANDROID__
// inline static const char* APK = "/apk/";
// // Macro for checking if a given pathname is inside APK's assets directory
// #  define SE_IS_ASSET(p) p.starts_with(APK)
// // Macro for truncating the APK prefix string from the asset pathname and at the same time patching the directory name components (see custom_rules.xml)
// #  ifdef ASSET_DIR_INDICATOR
// #    define SE_ASSET(p) p.substr(5).replaced("/", ASSET_DIR_INDICATOR "/").c_str()
// #  else
// #     define SE_ASSET(p) p.substr(5).c_str()
// #  endif
// #endif

/// File open mode.
enum FileMode
{
    FILE_READ = 0,
    FILE_WRITE,
    FILE_READWRITE
};

/// %File opened either through the filesystem or from within a package file.
class File : public AbstractFile
{

public:
    /// Construct.
    explicit File();
    /// Construct and open a filesystem file.
    File(const String& fileName, FileMode mode = FILE_READ);
    /// Construct and open from a package file.
    File(PackageFile* package, const String& fileName);
    /// Destruct. Close the file if open.
    ~File() override;

    /// Read bytes from the file. Return number of bytes actually read.
    unsigned Read(void* dest, unsigned size) override;
    /// Set position from the beginning of the file.
    unsigned Seek(unsigned position) override;
    ///
    void SeekCur(unsigned offset);
    int SeekSet(int offset);
    int SeekEnd(int offset);

    /// Write bytes to the file. Return number of bytes actually written.
    unsigned Write(const void* data, unsigned size) override;

    int getc() const;

    template<class T>
    int WriteR(T value) {
        T buf;
        const auto *s = (const unsigned char*)&value;
        unsigned char *d = (unsigned char*)&buf + sizeof(T) - 1;
        for(int i = 0; i < (int)sizeof(T); i++) {
            *d-- = *s++;
        }
        return Write(&buf, sizeof(T)*1);
    }

    /// Return the file name.
    String GetName() const override { return fileName_; }

    /// Return absolute file name in file system.
    const String& GetAbsoluteName() const override { return absoluteFileName_; }
    /// Return a checksum of the file contents using the SDBM hash algorithm.
    unsigned GetChecksum() override;

    /// Open a filesystem file. Return true if successful.
    bool Open(const String& fileName, FileMode mode = FILE_READ);
    /// Open from within a package file. Return true if successful.
    bool Open(PackageFile* package, const String& fileName);
    /// Close the file.
    void Close() override;
    /// Flush any buffered output to the file.
    void Flush();
    /// Change the file name. Used by the resource system.
    void SetName(const String& name) override;

    /// Return the open mode.
    FileMode GetMode() const { return mode_; }

    /// Return whether is open.
    bool IsOpen() const override;

    /// Return the file handle.
    void* GetHandle() const { return handle_; }

    /// Return whether the file originates from a package.
    bool IsPackaged() const { return offset_ != 0; }

        /// Reads a binary file to buffer.
    void ReadBinary(std::vector<unsigned char>& buffer);

    /// Reads a binary file to buffer.
    std::vector<unsigned char> ReadBinary() { std::vector<unsigned char> retValue; ReadBinary(retValue); return retValue; }

    /// Reads a text file, ensuring data from file is 0 terminated
    virtual void ReadText(String& text);

    /// Reads a text file, ensuring data from file is 0 terminated
    virtual String ReadText() { String retValue; ReadText(retValue); return retValue; }

#ifdef __ANDROID__
    inline static const unsigned READ_BUFFER_SIZE = 32768;
#endif
    inline static const unsigned SKIP_BUFFER_SIZE = 1024;

private:
    /// Open file internally using either C standard IO functions or SDL RWops for Android asset files. Return true if successful.
    bool OpenInternal(const String& fileName, FileMode mode, bool fromPackage = false);
    /// Perform the file read internally using either C standard IO functions or SDL RWops for Android asset files. Return true if successful. This does not handle compressed package file reading.
    bool ReadInternal(void* dest, unsigned size);
    /// Seek in file internally using either C standard IO functions or SDL RWops for Android asset files.
    void SeekInternal(unsigned newPosition);


    /// File name.
    String fileName_;
    /// Absolute file name.
    String absoluteFileName_;
    /// Open mode.
    FileMode mode_;
    /// File handle.
    void* handle_;
#ifdef __ANDROID__
    /// SDL RWops context for Android asset loading.
    SDL_RWops* assetHandle_;
#endif
    /// Read buffer for Android asset or compressed file loading.
    std::shared_ptr<unsigned char> readBuffer_;
    /// Decompression input buffer for compressed file loading.
    std::shared_ptr<unsigned char> inputBuffer_;
    /// Read buffer position.
    unsigned readBufferOffset_;
    /// Bytes in the current read buffer.
    unsigned readBufferSize_;
    /// Start position within a package file, 0 for regular files.
    unsigned offset_;
    /// Content checksum.
    unsigned checksum_;
    /// Compression flag.
    bool compressed_;
    /// Synchronization needed before read -flag.
    bool readSyncNeeded_;
    /// Synchronization needed before write -flag.
    bool writeSyncNeeded_;

private:
#ifdef _WIN32
    inline static const wchar_t* openMode[] =
    {
        L"rb",
        L"wb",
        L"r+b",
        L"w+b"
    };
#else
    inline static const char* openMode[] =
    {
        "rb",
        "wb",
        "r+b",
        "w+b"
    };
#endif
};

using FilePtr = std::shared_ptr<File>;

}
