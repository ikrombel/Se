#include "BinaryArchive.h"

#include <cassert>
#include <limits>

namespace Se
{

    namespace {

        constexpr const char* blockGuardName = "<block guard>";

    }

    BinaryOutputArchiveBlock::BinaryOutputArchiveBlock(const char* name, ArchiveBlockType type, Serializer* parentSerializer, bool safe)
            : ArchiveBlockBase(name, type)
            , parentSerializer_(parentSerializer)
    {
        if (safe)
            outputBuffer_ = std::make_unique<VectorBuffer>();
    }

    void BinaryOutputArchiveBlock::Close(ArchiveBase& archive)
    {
        if (!outputBuffer_)
        {
            assert(!HasOpenInlineBlock());
            return;
        }

        const unsigned size = outputBuffer_->GetSize();
        if (parentSerializer_->WriteVLE(size))
        {
            if (parentSerializer_->Write(outputBuffer_->GetData(), size) == size)
                return;
        }

        throw archive.IOFailureException(blockGuardName);
    }

    Serializer* BinaryOutputArchiveBlock::GetSerializer()
    {
        if (outputBuffer_)
            return outputBuffer_.get();
        else
            return parentSerializer_;
    }

    BinaryOutputArchive::BinaryOutputArchive(Serializer& serializer)
            : BinaryArchiveBase<BinaryOutputArchiveBlock, false>()
            , serializer_(&serializer)
    {
    }

    String BinaryOutputArchive::GetName() const
    {
        if (Deserializer* deserializer = dynamic_cast<Deserializer*>(serializer_))
            return deserializer->GetName();
        return {};
    }

    unsigned BinaryOutputArchive::GetChecksum()
    {
        if (Deserializer* deserializer = dynamic_cast<Deserializer*>(serializer_))
            return deserializer->GetChecksum();
        return 0;
    }

    void BinaryOutputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
    {
        CheckBeforeBlock(name);

        if (stack_.empty())
        {
            Block block{ name, type, serializer_, safe };
            stack_.push_back(std::move(block));
            currentBlockSerializer_ = GetCurrentBlock().GetSerializer();
        }
        else
        {
            if (safe)
            {
                Block block{ name, type, GetCurrentBlock().GetSerializer(), safe };
                stack_.push_back(std::move(block));
                currentBlockSerializer_ = GetCurrentBlock().GetSerializer();
            }
            else
            {
                GetCurrentBlock().OpenInlineBlock();
            }
        }

        if (type == ArchiveBlockType::Array)
        {
            if (!currentBlockSerializer_->WriteVLE(sizeHint))
            {
                EndBlock();
                throw IOFailureException(blockGuardName);
            }
        }
    }

    void BinaryOutputArchive::EndBlock() noexcept
    {
        ArchiveBaseT::EndBlock();
        currentBlockSerializer_ = stack_.empty() ? nullptr : GetCurrentBlock().GetSerializer();
    }

    void BinaryOutputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
    {
        CheckBeforeElement(name);
        CheckResult(currentBlockSerializer_->Write(bytes, size) == size, name);
    }

    void BinaryOutputArchive::SerializeVLE(const char* name, unsigned& value)
    {
        CheckBeforeElement(name);
        CheckResult(currentBlockSerializer_->WriteVLE(value), name);
    }

// Generate serialization implementation (binary output)
#define SE_BINARY_OUT_IMPL(type, function) \
    void BinaryOutputArchive::Serialize(const char* name, type& value) \
    { \
        CheckBeforeElement(name); \
        CheckResult(currentBlockSerializer_->function(value), name); \
    }

    SE_BINARY_OUT_IMPL(bool, WriteBool);
    SE_BINARY_OUT_IMPL(signed char, WriteByte);
    SE_BINARY_OUT_IMPL(unsigned char, WriteUByte);
    SE_BINARY_OUT_IMPL(short, WriteShort);
    SE_BINARY_OUT_IMPL(unsigned short, WriteUShort);
    SE_BINARY_OUT_IMPL(int, WriteInt);
    SE_BINARY_OUT_IMPL(unsigned int, WriteUInt);
    SE_BINARY_OUT_IMPL(long long, WriteInt64);
    SE_BINARY_OUT_IMPL(unsigned long long, WriteUInt64);
    SE_BINARY_OUT_IMPL(float, WriteFloat);
    SE_BINARY_OUT_IMPL(double, WriteDouble);
    SE_BINARY_OUT_IMPL(String, WriteString);

#undef SE_BINARY_OUT_IMPL

    BinaryInputArchiveBlock::BinaryInputArchiveBlock(const char* name, ArchiveBlockType type,
                                                     Deserializer* deserializer, bool safe, unsigned nextElementPosition)
            : ArchiveBlockBase(name, type)
            , deserializer_(deserializer)
            , safe_(safe)
            , nextElementPosition_(nextElementPosition)
    {
        if (safe_)
        {
            blockSize_ = deserializer_->ReadVLE();
            blockOffset_ = deserializer_->GetPosition();
            nextElementPosition_ = std::min(blockOffset_ + blockSize_, deserializer_->GetSize());
        }
    }

    void BinaryInputArchiveBlock::Close(ArchiveBase& archive)
    {
        if (safe_)
        {
            assert(nextElementPosition_ != std::numeric_limits<unsigned>::max());
            const unsigned currentPosition = deserializer_->GetPosition();
            if (nextElementPosition_ != currentPosition)
                deserializer_->Seek(nextElementPosition_);
        }
    }

    BinaryInputArchive::BinaryInputArchive(Deserializer& deserializer)
            : BinaryArchiveBase<BinaryInputArchiveBlock, true>()
            , deserializer_(&deserializer)
    {
    }

    void BinaryInputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
    {
        CheckBeforeBlock(name);

        if (stack_.empty())
        {
            Block frame{ name, type, deserializer_, safe, std::numeric_limits<unsigned>::max() };
            stack_.push_back(frame);
        }
        else
        {
            if (safe)
            {
                Block blockFrame{ name, type, deserializer_, safe, GetCurrentBlock().GetNextElementPosition() };
                stack_.push_back(blockFrame);
            }
            else
            {
                GetCurrentBlock().OpenInlineBlock();
            }
        }

        if (type == ArchiveBlockType::Array)
        {
            sizeHint = deserializer_->ReadVLE();
            if (deserializer_->IsEof() && sizeHint != 0)
            {
                EndBlock();
                throw IOFailureException(blockGuardName);
            }
        }
    }

    void BinaryInputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
    {
        CheckBeforeElement(name);
        CheckResult(deserializer_->Read(bytes, size) == size, name);
    }

    void BinaryInputArchive::SerializeVLE(const char* name, unsigned& value)
    {
        CheckBeforeElement(name);
        value = deserializer_->ReadVLE();
        CheckResult(true, name);
    }

// Generate serialization implementation (binary input)
#define SE_BINARY_IN_IMPL(type, function) \
    void BinaryInputArchive::Serialize(const char* name, type& value) \
    { \
        CheckBeforeElement(name); \
        value = deserializer_->function(); \
        CheckResult(true, name); \
    }

    SE_BINARY_IN_IMPL(bool, ReadBool);
    SE_BINARY_IN_IMPL(signed char, ReadByte);
    SE_BINARY_IN_IMPL(unsigned char, ReadUByte);
    SE_BINARY_IN_IMPL(short, ReadShort);
    SE_BINARY_IN_IMPL(unsigned short, ReadUShort);
    SE_BINARY_IN_IMPL(int, ReadInt);
    SE_BINARY_IN_IMPL(unsigned int, ReadUInt);
    SE_BINARY_IN_IMPL(long long, ReadInt64);
    SE_BINARY_IN_IMPL(unsigned long long, ReadUInt64);
    SE_BINARY_IN_IMPL(float, ReadFloat);
    SE_BINARY_IN_IMPL(double, ReadDouble);
    SE_BINARY_IN_IMPL(String, ReadString);

#undef SE_BINARY_IN_IMPL

}