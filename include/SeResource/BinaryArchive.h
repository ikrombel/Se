
#pragma once

#include <SeArc/ArchiveBase.hpp>
#include <Se/IO/Deserializer.hpp>
#include <Se/IO/Serializer.hpp>
#include <Se/IO/VectorBuffer.h>

namespace Se
{

/// Base archive for binary serialization.
template <class BlockType, bool IsInputBool>
class BinaryArchiveBase : public ArchiveBaseT<BlockType, IsInputBool, false>
{
protected:
    using ArchiveBaseT<BlockType, IsInputBool, false>::ArchiveBaseT;

    void CheckResult(bool result, const char* elementName) const
    {
        if (!result)
            throw this->IOFailureException(elementName);
    }
};

/// Binary output archive block.
class BinaryOutputArchiveBlock : public ArchiveBlockBase
{
public:
    BinaryOutputArchiveBlock(const char* name, ArchiveBlockType type, Serializer* parentSerializer, bool safe);
    Serializer* GetSerializer();

    bool IsUnorderedAccessSupported() const { return false; }
    bool HasElementOrBlock(const char* name) const { return false; }
    void Close(ArchiveBase& archive);

private:
    /// @name For safe blocks only
    /// @{
    std::unique_ptr<VectorBuffer> outputBuffer_;
    Serializer* parentSerializer_{};
    /// @}
};

/// Binary output archive.
class BinaryOutputArchive : public BinaryArchiveBase<BinaryOutputArchiveBlock, false>
{
    friend class BinaryOutputArchiveBlock;

public:
    BinaryOutputArchive(Serializer& serializer);
    
    /// @name Archive implementation
    /// @{
    String GetName() const final;
    unsigned GetChecksum() final;
    
    void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) final;
    void EndBlock() noexcept final;
    
    void Serialize(const char* name, bool& value) final;
    void Serialize(const char* name, signed char& value) final;
    void Serialize(const char* name, unsigned char& value) final;
    void Serialize(const char* name, short& value) final;
    void Serialize(const char* name, unsigned short& value) final;
    void Serialize(const char* name, int& value) final;
    void Serialize(const char* name, unsigned int& value) final;
    void Serialize(const char* name, long long& value) final;
    void Serialize(const char* name, unsigned long long& value) final;
    void Serialize(const char* name, float& value) final;
    void Serialize(const char* name, double& value) final;
    void Serialize(const char* name, String& value) final;
    
    void SerializeBytes(const char* name, void* bytes, unsigned size) final;
    void SerializeVLE(const char* name, unsigned& value) final;
    /// @}

private:
    Serializer* serializer_{};
    /// Serializer used within currently open block.
    Serializer* currentBlockSerializer_{};
};

/// Binary input archive block.
class BinaryInputArchiveBlock : public ArchiveBlockBase {
public:
    BinaryInputArchiveBlock(
            const char *name, ArchiveBlockType type, Deserializer *deserializer, bool safe,
            unsigned nextElementPosition);

    unsigned GetNextElementPosition() const { return nextElementPosition_; }

    bool IsUnorderedAccessSupported() const { return false; }

    bool HasElementOrBlock(const char *name) const { return false; }

    void Close(ArchiveBase &archive);

private:
    Deserializer *deserializer_{};
/// Whether the block is safe.
    bool safe_{};

/// @name For safe blocks only
/// @{
    unsigned blockOffset_{};
    unsigned blockSize_{};
    unsigned nextElementPosition_{};
/// @}
};

/// Binary input archive.
class BinaryInputArchive : public BinaryArchiveBase<BinaryInputArchiveBlock, true> {
public:
    BinaryInputArchive(Deserializer &deserializer);

    /// @name Archive implementation
    /// @{
    String GetName() const final { return deserializer_->GetName(); }

    unsigned GetChecksum() final { return deserializer_->GetChecksum(); }

    void BeginBlock(const char *name, unsigned &sizeHint, bool safe, ArchiveBlockType type) final;

    void Serialize(const char *name, bool &value) final;

    void Serialize(const char *name, signed char &value) final;

    void Serialize(const char *name, unsigned char &value) final;

    void Serialize(const char *name, short &value) final;

    void Serialize(const char *name, unsigned short &value) final;

    void Serialize(const char *name, int &value) final;

    void Serialize(const char *name, unsigned int &value) final;

    void Serialize(const char *name, long long &value) final;

    void Serialize(const char *name, unsigned long long &value) final;

    void Serialize(const char *name, float &value) final;

    void Serialize(const char *name, double &value) final;

    void Serialize(const char *name, String &value) final;

    void SerializeBytes(const char *name, void *bytes, unsigned size) final;

    void SerializeVLE(const char *name, unsigned &value) final;
    /// @}

private:
    /// Deserializer.
    Deserializer *deserializer_{};

};

}