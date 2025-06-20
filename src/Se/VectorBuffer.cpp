#include <Se/IO/VectorBuffer.h>

namespace Se
{

VectorBuffer::VectorBuffer() = default;

VectorBuffer::VectorBuffer(const ByteVector& data)
{
    SetData(data);
}

VectorBuffer::VectorBuffer(const void* data, std::size_t size)
{
    SetData(data, size);
}

VectorBuffer::VectorBuffer(Deserializer& source, std::size_t size)
{
    SetData(source, size);
}

std::size_t VectorBuffer::Read(void* dest, std::size_t size)
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

std::size_t VectorBuffer::Seek(std::size_t position)
{
    if (position > size_)
        position = size_;

    position_ = position;
    return position_;
}

std::size_t VectorBuffer::Write(const void* data, std::size_t size)
{
    if (!size)
        return 0;

    if (size + position_ > size_)
    {
        size_ = size + position_;
        buffer_.resize(size_);
    }

    auto* srcPtr = (unsigned char*)data;
    unsigned char* destPtr = &buffer_[position_];
    position_ += size;

    memcpy(destPtr, srcPtr, size);

    return size;
}

void VectorBuffer::SetData(const ByteVector& data)
{
    buffer_ = data;
    position_ = 0;
    size_ = data.size();
}

void VectorBuffer::SetData(const void* data, std::size_t size)
{
    if (!data)
        size = 0;

    buffer_.resize(size);
    if (size)
        memcpy(&buffer_[0], data, size);

    position_ = 0;
    size_ = size;
}

void VectorBuffer::SetData(Deserializer& source, std::size_t size)
{
    buffer_.resize(size);
    std::size_t actualSize = source.Read(&buffer_[0], size);
    if (actualSize != size)
        buffer_.resize(actualSize);

    position_ = 0;
    size_ = actualSize;
}

void VectorBuffer::Clear()
{
    buffer_.clear();
    position_ = 0;
    size_ = 0;
}

void VectorBuffer::Resize(std::size_t size)
{
    buffer_.resize(size);
    size_ = size;
    if (position_ > size_)
        position_ = size_;
}

}
