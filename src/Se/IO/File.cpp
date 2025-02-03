#include "File.h"

#include <Se/Console.hpp>
#include <Se/IO/PackageFile.h>

#ifdef HAVE_LZ4
#include <LZ4/lz4.h>
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef _MSC_VER
#define fseeko64 _fseeki64
#endif

namespace Se {

File::File() :
    mode_(FILE_READ),
    handle_(nullptr),
#ifdef __ANDROID__
    assetHandle_(0),
#endif
    readBufferOffset_(0),
    readBufferSize_(0),
    offset_(0),
    checksum_(0),
    compressed_(false),
    readSyncNeeded_(false),
    writeSyncNeeded_(false)
{
}

File::File(const String& fileName, FileMode mode) :
    mode_(FILE_READ),
    handle_(nullptr),
#ifdef __ANDROID__
    assetHandle_(0),
#endif
    readBufferOffset_(0),
    readBufferSize_(0),
    offset_(0),
    checksum_(0),
    compressed_(false),
    readSyncNeeded_(false),
    writeSyncNeeded_(false)
{
    Open(fileName, mode);
}

File::File(PackageFile* package, const String& fileName) :
    mode_(FILE_READ),
    handle_(nullptr),
#ifdef __ANDROID__
    assetHandle_{nullptr},
#endif
    readBufferOffset_(0),
    readBufferSize_(0),
    offset_(0),
    checksum_(0),
    compressed_(false),
    readSyncNeeded_(false),
    writeSyncNeeded_(false)
{
    Open(package, fileName);
}

File::~File()
{
    Close();
}

bool File::Open(const String& fileName, FileMode mode)
{
    return OpenInternal(fileName, mode);
}

inline bool File::Open(PackageFile* package, const String& fileName)
{
    if (!package)
        return false;

    const PackageEntry* entry = package->GetEntry(fileName);
    if (!entry)
        return false;

    bool success = OpenInternal(package->GetName(), FILE_READ, true);
    if (!success)
    {
        SE_LOG_ERROR("Could not open package file {}", fileName);
        return false;
    }

    fileName_ = fileName;
    offset_ = entry->offset_;
    checksum_ = entry->checksum_;
    size_ = entry->size_;
    compressed_ = package->IsCompressed();

    // Seek to beginning of package entry's file data
    SeekInternal(offset_);
    return true;
}

unsigned File::Read(void* dest, unsigned size)
{
    if (!IsOpen())
    {
        // If file not open, do not log the error further here to prevent spamming the stderr stream
        return 0;
    }

    if (mode_ == FILE_WRITE)
    {
        SE_LOG_ERROR("File not opened for reading");
        return 0;
    }

    if (size + position_ > size_)
        size = size_ - position_;
    if (!size)
        return 0;

#ifdef __ANDROID__
    if (assetHandle_ && !compressed_)
    {
        // If not using a compressed package file, buffer file reads on Android for better performance
        if (!readBuffer_)
        {
            readBuffer_ = std::shared_ptr<unsigned char>(new unsigned char[READ_BUFFER_SIZE], std::default_delete<unsigned char[]>());
            readBufferOffset_ = 0;
            readBufferSize_ = 0;
        }

        unsigned sizeLeft = size;
        unsigned char* destPtr = (unsigned char*)dest;

        while (sizeLeft)
        {
            if (readBufferOffset_ >= readBufferSize_)
            {
                readBufferSize_ = std::min(size_ - position_, READ_BUFFER_SIZE);
                readBufferOffset_ = 0;
                ReadInternal(readBuffer_.get(), readBufferSize_);
            }

            unsigned copySize = std::min((readBufferSize_ - readBufferOffset_), sizeLeft);
            memcpy(destPtr, readBuffer_.get() + readBufferOffset_, copySize);
            destPtr += copySize;
            sizeLeft -= copySize;
            readBufferOffset_ += copySize;
            position_ += copySize;
        }

        return size;
    }
#endif

#ifdef HAVE_LZ4
    if (compressed_)
    {
        unsigned sizeLeft = size;
        auto* destPtr = (unsigned char*)dest;

        while (sizeLeft)
        {
            if (!readBuffer_ || readBufferOffset_ >= readBufferSize_)
            {
                unsigned char blockHeaderBytes[4];
                ReadInternal(blockHeaderBytes, sizeof blockHeaderBytes);

                MemoryBuffer blockHeader(&blockHeaderBytes[0], sizeof blockHeaderBytes);
                unsigned unpackedSize = blockHeader.ReadUShort();
                unsigned packedSize = blockHeader.ReadUShort();

                if (!readBuffer_)
                {
                    readBuffer_ = std::shared_ptr<unsigned char>(new unsigned char[unpackedSize], std::default_delete<unsigned char[]>());
                    inputBuffer_ = std::shared_ptr<unsigned char>(new unsigned char[LZ4_compressBound(unpackedSize)], std::default_delete<unsigned char[]>());
                }

                /// \todo Handle errors
                ReadInternal(inputBuffer_.get(), packedSize);
                LZ4_decompress_fast((const char*)inputBuffer_.get(), (char*)readBuffer_.get(), unpackedSize);

                readBufferSize_ = unpackedSize;
                readBufferOffset_ = 0;
            }

            unsigned copySize = std::min((readBufferSize_ - readBufferOffset_), sizeLeft);
            memcpy(destPtr, readBuffer_.get() + readBufferOffset_, copySize);
            destPtr += copySize;
            sizeLeft -= copySize;
            readBufferOffset_ += copySize;
            position_ += copySize;
        }

        return size;
    }
#endif

    // Need to reassign the position due to internal buffering when transitioning from writing to reading
    if (readSyncNeeded_)
    {
        SeekInternal(position_ + offset_);
        readSyncNeeded_ = false;
    }

    if (!ReadInternal(dest, size))
    {
        // Return to the position where the read began
        SeekInternal(position_ + offset_);
        SE_LOG_ERROR("Error while reading from file " + GetName());
        return 0;
    }

    writeSyncNeeded_ = true;
    position_ += size;
    return size;
}

unsigned File::Seek(unsigned position)
{
    if (!IsOpen())
    {
        // If file not open, do not log the error further here to prevent spamming the stderr stream
        return 0;
    }

    // Allow sparse seeks if writing
    if (mode_ == FILE_READ && position > size_)
        position = size_;

    if (compressed_)
    {
        // Start over from the beginning
        if (position == 0)
        {
            position_ = 0;
            readBufferOffset_ = 0;
            readBufferSize_ = 0;
            SeekInternal(offset_);
        }
        // Skip bytes
        else if (position >= position_)
        {
            unsigned char skipBuffer[SKIP_BUFFER_SIZE];
            while (position > position_)
                Read(skipBuffer, std::min(position - position_, SKIP_BUFFER_SIZE));
        }
         else
            SE_LOG_ERROR("Seeking backward in a compressed file is not supported");

        return position_;
    }

    SeekInternal(position + offset_);
    position_ = position;
    readSyncNeeded_ = false;
    writeSyncNeeded_ = false;
    return position_;
}

unsigned File::Write(const void* data, unsigned size)
{
    if (!IsOpen())
    {
        // If file not open, do not log the error further here to prevent spamming the stderr stream
        return 0;
    }

    if (mode_ == FILE_READ)
    {
        SE_LOG_ERROR("File not opened for writing");
        return 0;
    }

    if (!size)
        return 0;

    // Need to reassign the position due to internal buffering when transitioning from reading to writing
    if (writeSyncNeeded_)
    {
        fseek((FILE*)handle_, (long)position_ + offset_, SEEK_SET);
        writeSyncNeeded_ = false;
    }

    if (fwrite(data, size, 1, (FILE*)handle_) != 1)
    {
        // Return to the position where the write began
        fseek((FILE*)handle_, (long)position_ + offset_, SEEK_SET);
        SE_LOG_ERROR("Error while writing to file " + GetName());
        return 0;
    }

    readSyncNeeded_ = true;
    position_ += size;
    if (position_ > size_)
        size_ = position_;

    return size;
}

unsigned File::GetChecksum()
{
    if (offset_ || checksum_)
        return checksum_;
#ifdef __ANDROID__
    if ((!handle_ && !assetHandle_) || mode_ == FILE_WRITE)
#else
    if (!handle_ || mode_ == FILE_WRITE)
#endif
        return 0;

//    SE_PROFILE("CalculateFileChecksum");

    unsigned oldPos = position_;
    checksum_ = 0;

    Seek(0);
    while (!IsEof())
    {
        unsigned char block[1024];
        unsigned readBytes = Read(block, 1024);
        for (unsigned i = 0; i < readBytes; ++i)
            checksum_ = Se::SDBMHash(checksum_, block[i]);
    }

    Seek(oldPos);
    return checksum_;
}

void File::Close()
{
#ifdef __ANDROID__
    if (assetHandle_)
    {
        SDL_RWclose(assetHandle_);
        assetHandle_ = 0;
    }
#endif

    readBuffer_.reset();
    inputBuffer_.reset();

    if (handle_)
    {
        fclose((FILE*)handle_);
        handle_ = nullptr;
        position_ = 0;
        size_ = 0;
        offset_ = 0;
        checksum_ = 0;
    }
}

void File::Flush()
{
    if (handle_)
        fflush((FILE*)handle_);
}

void File::SetName(const String& name)
{
    fileName_ = name;
}

bool File::IsOpen() const
{
#ifdef __ANDROID__
    return handle_ != 0 || assetHandle_ != 0;
#else
    return handle_ != nullptr;
#endif
}

bool File::OpenInternal(const String& fileName, FileMode mode, bool fromPackage)
{
    Close();

    compressed_ = false;
    readSyncNeeded_ = false;
    writeSyncNeeded_ = false;

    // auto fileSystem = FileSystem::Get();
    // if (!fileSystem.CheckAccess(GetPath(fileName)))
    // {
    //     SE_LOG_ERROR("Access denied to {}", fileName);
    //     return false;
    // }

    if (fileName.empty())
    {
        SE_LOG_ERROR("Could not open file with empty name");
        return false;
    }

#ifdef __ANDROID__
    if (SE_IS_ASSET(fileName))
    {
        if (mode != FILE_READ)
        {
            SE_LOG_ERROR("Only read mode is supported for Android asset files");
            return false;
        }

        assetHandle_ = SDL_RWFromFile(SE_ASSET(fileName), "rb");
        if (!assetHandle_)
        {
            SE_LOG_ERROR("Could not open Android asset file {}", fileName.c_str());
            return false;
        }
        else
        {
            fileName_ = fileName;
            absoluteFileName_ = fileName;
            mode_ = mode;
            position_ = 0;
            if (!fromPackage)
            {
                size_ = SDL_RWsize(assetHandle_);
                offset_ = 0;
            }
            checksum_ = 0;
            return true;
        }
    }
#endif

#ifdef _WIN32
    handle_ = _wfopen(GetWideNativePath(fileName).c_str(), openMode[mode]);
#else
    handle_ = fopen(GetNativePath(fileName).c_str(), openMode[mode]);
#endif

    // If file did not exist in readwrite mode, retry with write-update mode
    if (mode == FILE_READWRITE && !handle_)
    {
#ifdef _WIN32
        handle_ = _wfopen(GetWideNativePath(fileName).c_str(), openMode[mode + 1]);
#else
        handle_ = fopen(GetNativePath(fileName).c_str(), openMode[mode + 1]);
#endif
    }

    if (!handle_)
    {
        SE_LOG_ERROR("Could not open file {}", fileName);
        return false;
    }

    if (!fromPackage)
    {
        fseek((FILE*)handle_, 0, SEEK_END);
        long size = ftell((FILE*)handle_);
        fseek((FILE*)handle_, 0, SEEK_SET);
        if (size > std::numeric_limits<unsigned>::max())
        {
            SE_LOG_ERROR("Could not open file {} which is larger than 4GB", fileName);
            Close();
            size_ = 0;
            return false;
        }
        size_ = (unsigned)size;
        offset_ = 0;
    }

    fileName_ = fileName;
    absoluteFileName_ = fileName;
    mode_ = mode;
    position_ = 0;
    checksum_ = 0;

    return true;
}

bool File::ReadInternal(void* dest, unsigned size)
{
#ifdef __ANDROID__
    if (assetHandle_)
    {
        return SDL_RWread(assetHandle_, dest, size, 1) == 1;
    }
    else
#endif
        return fread(dest, size, 1, (FILE*)handle_) == 1;
}

void File::SeekInternal(unsigned newPosition)
{
#ifdef __ANDROID__
    if (assetHandle_)
    {
        SDL_RWseek(assetHandle_, newPosition, SEEK_SET);
        // Reset buffering after seek
        readBufferOffset_ = 0;
        readBufferSize_ = 0;
    }
    else
        fseek((FILE*)handle_, newPosition, SEEK_SET);
#else      
#  ifdef _MSC_VER
    _fseeki64((FILE*)handle_, newPosition, SEEK_SET);
#  else
    fseeko64((FILE*)handle_, newPosition, SEEK_SET);
#  endif
#endif
}

void File::ReadBinary(std::vector<unsigned char>& buffer)
{
    buffer.clear();

    if (!size_)
        return;

    buffer.resize(size_);

    Read(static_cast<void*>(buffer.data()), size_);
}

void File::ReadText(String& text)
{
    text.clear();

    if (!size_)
        return;

    text.resize(size_);

    Read(static_cast<void*>(&text[0]), size_);
}

void File::SeekCur(unsigned offset)
{
#ifdef __ANDROID__
    if (assetHandle_)
    {
        SDL_RWseek(assetHandle_, offset, SEEK_CUR);
        // Reset buffering after seek
        readBufferOffset_ = 0;
        readBufferSize_ = 0;
    }
    else
        fseek((FILE*)handle_, offset, SEEK_CUR);
#else
#  ifdef _MSC_VER
    _fseeki64((FILE*)handle_, offset, SEEK_CUR);
#  else
    fseeko64((FILE*)handle_, offset, SEEK_CUR);
#  endif
#endif
}

int File::SeekSet(int offset) {
    assert((FILE*)handle_ != nullptr && "File::SeekCur(): file is not opened");

#ifdef __ANDROID__
    if (assetHandle_) {
        return SDL_RWseek(assetHandle_, offset, SEEK_SET) == 0;
    }
    
    return fseek((FILE*)handle_, offset, SEEK_SET);
#else
    return (fseeko64((FILE*)handle_, (long)offset, SEEK_SET) == 0);
#endif
}

int File::SeekEnd(int offset) {
    assert((FILE*)handle_ != nullptr && "File::SeekEnd(): file is not opened");
    
#ifdef __ANDROID__
    if (assetHandle_) {
        return SDL_RWseek(assetHandle_, offset, SEEK_END);
    }
    else 
        return fseek((FILE*)handle_, offset, SEEK_END);
#else
    return fseeko64((FILE*)handle_, (long)offset, SEEK_END);
#endif
}

int File::getc() const {
    assert((FILE*)handle_ != nullptr && "File::getc(): file is not opened");
    return fgetc((FILE*)handle_);
}



}