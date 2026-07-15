#pragma once

// #include <GFrost/Container/VectorBase.h>
#include <Se/IO/AbstractFile.hpp>

namespace Se
{

using ByteVector = std::vector<unsigned char>;

/// Dynamically sized buffer that can be read and written to as a stream.
class VectorBuffer : public AbstractFile
{
public:
    /// Construct an empty buffer.
    VectorBuffer();
    /// Construct from another buffer.
    explicit VectorBuffer(const ByteVector& data);
    /// Construct from a memory area.
    VectorBuffer(const void* data, std::size_t size);
    /// Construct from a stream.
    VectorBuffer(Deserializer& source, std::size_t size);

    /// Read bytes from the buffer. Return number of bytes actually read.
    std::size_t Read(void* dest, std::size_t size) override;
    /// Set position from the beginning of the buffer. Return actual new position.
    std::size_t Seek(std::size_t position) override;
    /// Write bytes to the buffer. Return number of bytes actually written.
    std::size_t Write(const void* data, std::size_t size) override;

    /// Set data from another buffer.
    void SetData(const ByteVector& data);
    /// Set data from a memory area.
    void SetData(const void* data, std::size_t size);
    /// Set data from a stream.
    void SetData(Deserializer& source, std::size_t size);
    /// Reset to zero size.
    void Clear();
    /// Set size.
    void Resize(std::size_t size);

    /// Return data.
    const unsigned char* GetData() const { return size_ ? &buffer_[0] : nullptr; }

    /// Return non-const data.
    unsigned char* GetModifiableData() { return size_ ? &buffer_[0] : nullptr; }

    /// Return the buffer.
    const ByteVector& GetBuffer() const { return buffer_; }

private:
    /// Dynamic data buffer.
    ByteVector buffer_;
};

}
