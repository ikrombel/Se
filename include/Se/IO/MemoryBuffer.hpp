// Copyright (c) 2008-2019 the GFrost project.

#pragma once

#include <Se/IO/AbstractFile.hpp>

namespace Se
{

/// Memory area that can be read and written to as a stream.
class MemoryBuffer : public AbstractFile
{
public:
    /// Construct with a pointer and size.
    MemoryBuffer(void* data, unsigned size)
            : AbstractFile(size)
            , buffer_((unsigned char*)data)
            , readOnly_(false)
    {
        if (!buffer_)
                size_ = 0;
    }
    /// Construct as read-only with a pointer and size.
    MemoryBuffer(const void* data, unsigned size)
            : AbstractFile(size)
            , buffer_((unsigned char*)data)
            , readOnly_(true) 
    {

    }
    /// Construct as read-only from string.
    explicit MemoryBuffer(const String& text)
            : AbstractFile(static_cast<unsigned>(text.length()))
            , buffer_(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(text.c_str())))
            , readOnly_(true)
    {

    }
    /// Construct from a vector, which must not go out of scope before MemoryBuffer.
    explicit MemoryBuffer(std::vector<unsigned char>& data)
            : AbstractFile(data.size())
            , buffer_(data.data())
            , readOnly_(false)
    {

    }

    /// Construct from a read-only vector, which must not go out of scope before MemoryBuffer.
    explicit MemoryBuffer(const std::vector<unsigned char>& data)
            : AbstractFile(data.size())
            , buffer_(const_cast<unsigned char*>(data.data()))
            , readOnly_(true)
    {

    }
    // /// Construct from a vector buffer, which must not go out of scope before MemoryBuffer.
    // explicit MemoryBuffer(VectorBuffer& data);
    // /// Construct from a read-only vector buffer, which must not go out of scope before MemoryBuffer.
    // explicit MemoryBuffer(const VectorBuffer& data);

    ~MemoryBuffer() override {
        // if (buffer_)
        //     delete[] buffer_;
    }

    /// Read bytes from the memory area. Return number of bytes actually read.
    unsigned Read(void* dest, unsigned size) override
    {
        if (size + position_ > size_)
            size = size_ - position_;
        if (!size)
            return 0;

        unsigned char* srcPtr = &buffer_[position_];
        auto* destPtr = (unsigned char*)dest;
        position_ += size;

        memcpy(destPtr, srcPtr, size);

        return size;
    }
    /// Set position from the beginning of the memory area. Return actual new position.
    unsigned Seek(unsigned position) override
    {
        if (position > size_)
            position = size_;

        position_ = position;
        return position_;
    }

    /// Write bytes to the memory area.
    unsigned Write(const void* data, unsigned size) override 
    {
        if (size + position_ > size_)
            size = size_ - position_;
        if (!size)
            return 0;

        auto* srcPtr = (unsigned char*)data;
        unsigned char* destPtr = &buffer_[position_];
        position_ += size;

        memcpy(destPtr, srcPtr, size);

        return size;
    }

    /// Return memory area.
    unsigned char* GetData() { return buffer_; }
    const unsigned char* GetData() const { return buffer_; }

    /// Return whether buffer is read-only.
    bool IsReadOnly() { return readOnly_; }

protected:
    /// Pointer to the memory area.
    unsigned char* buffer_;
    /// Read-only flag.
    bool readOnly_;
};

// class MemoryBufferFile : public MemoryBuffer
// {

//     ~MemoryBufferFile() override {
//         if (buffer_)
//             delete[] buffer_;
//     }

// };

}
