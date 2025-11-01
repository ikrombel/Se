
#include <SeResource/BinaryArchive.h>

#include <Se/IO/Deserializer.hpp>
#include <Se/IO/File.h>
#include <Se/Console.hpp>
#include <Se/IO/MemoryBuffer.hpp>
#include <Se/IO/Serializer.hpp>

#include "BinaryFile.h"


namespace Se
{

BinaryFile::BinaryFile()
    : Resource("BinaryFile")
{

}

BinaryFile::~BinaryFile() = default;

// void BinaryFile::RegisterObject(Context* context)
// {
//     context->AddFactoryReflection<BinaryFile>();
// }

bool BinaryFile::BeginLoad(Deserializer& source)
{
    source.Seek(0);
    buffer_.SetData(source, source.GetSize());
    SetMemoryUse(buffer_.GetBuffer().capacity());
    return true;
}

bool BinaryFile::Save(Serializer& dest) const
{
    if (dest.Write(buffer_.GetData(), buffer_.GetSize()) != buffer_.GetSize())
    {
        SE_LOG_ERROR("Can not save binary file {}", GetName());
        return false;
    }

    return true;
}

bool BinaryFile::SaveObjectCallback(const std::function<void(Archive&)> serializeValue)
{
    try {
        buffer_.Clear();
        BinaryOutputArchive archive{AsSerializer()};
        serializeValue(archive);
        return true;
    }
    catch (const ArchiveException& e)
    {
        buffer_.Clear();
        SE_LOG_ERROR("Failed to save object to binary: {}", e.what());
        return false;
    }
}

bool BinaryFile::LoadObjectCallback(const std::function<void(Archive&)> serializeValue) const
{
    try
    {
        MemoryBuffer readBuffer{buffer_.GetBuffer()};
        BinaryInputArchive archive{readBuffer};
        serializeValue(archive);
        return true;
    }
    catch (const ArchiveException& e)
    {
        SE_LOG_ERROR("Failed to load object from binary: {}", e.what());
        return false;
    }
}

void BinaryFile::Clear()
{
    buffer_.Clear();
}

void BinaryFile::SetData(const ByteVector& data)
{
    buffer_.SetData(data);
    SetMemoryUse(buffer_.GetBuffer().capacity());
}

const ByteVector& BinaryFile::GetData() const
{
    return buffer_.GetBuffer();
}

String BinaryFile::GetText() const
{
    const unsigned char* data = buffer_.GetData();
    const unsigned size = buffer_.GetSize();
    return {reinterpret_cast<const char*>(data), size};
}

StringVector BinaryFile::ReadLines() const
{
    StringVector result;
    MemoryBuffer readBuffer{buffer_.GetBuffer()};
    while (!readBuffer.IsEof())
        result.push_back(readBuffer.ReadLine());
    return result;
}

}
