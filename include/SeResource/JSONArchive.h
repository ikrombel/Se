#pragma once

#include <SeMath/MathDefs.hpp>

#include <SeArc/ArchiveBase.hpp>
#include <SeResource/JSONFile.h>
#include <SeResource/JSONValue.h>
#include <Se/Math.hpp>

namespace Se
{

/// Base archive for JSON serialization.
template <class BlockType, bool IsInputBool>
class JSONArchiveBase : public ArchiveBaseT<BlockType, IsInputBool, true>
{
public:
    /// @name Archive implementation
    /// @{
    String GetName() const { return jsonFile_ ? jsonFile_->GetName() : ""; }
    /// @}

protected:
    explicit JSONArchiveBase(const JSONFile* jsonFile)
            : ArchiveBaseT<BlockType, IsInputBool, true>()
            , jsonFile_(jsonFile)
    {
    }

private:
    const JSONFile* jsonFile_{};
};

/// JSON output archive block. Internal.
struct JSONOutputArchiveBlock : public ArchiveBlockBase
{
public:
    JSONOutputArchiveBlock(const char* name, ArchiveBlockType type, Value* blockValue, unsigned sizeHint);
    Value& CreateElement(ArchiveBase& archive, const char* elementName);

    bool IsUnorderedAccessSupported() const { return type_ == ArchiveBlockType::Unordered; }
    bool HasElementOrBlock(const char* name) const { return false; }
    void Close(ArchiveBase& archive);

private:
    /// Block value.
    Value* blockValue_;

    /// Expected block size (for arrays).
    unsigned expectedElementCount_{ M_MAX_UNSIGNED };
    /// Number of elements in block.
    unsigned numElements_{};
};

class JSONOutputArchive : public JSONArchiveBase<JSONOutputArchiveBlock, false> {
public:
    /// Base type.
    using Base = JSONArchiveBase<JSONOutputArchiveBlock, true>;

    /// Construct from element.
    JSONOutputArchive(Value& value, JSONFile* jsonFile = nullptr)
            : JSONArchiveBase(jsonFile)
            , rootValue_(value)
    {
    }
    /// Construct from file.
    explicit JSONOutputArchive(JSONFile* jsonFile)
            : JSONArchiveBase(jsonFile)
            , rootValue_(jsonFile->GetRoot())
    {}

    /// @name Archive implementation
    /// @{
    void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) final;

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
    void CreateElement(const char* name, const Value& value);

    Value& rootValue_;
    String tempString_;
};

/// Archive stack frame helper.
struct JSONInputArchiveBlock : public ArchiveBlockBase
{
public:
    JSONInputArchiveBlock(const char* name, ArchiveBlockType type, const Value* value);
    /// Return size hint.
    unsigned GetSizeHint() const { return value_->Size(); }
    /// Read current child and move to the next one.
    const Value& ReadElement(ArchiveBase& archive, const char* elementName, const ArchiveBlockType* elementBlockType);

    bool IsUnorderedAccessSupported() const { return type_ == ArchiveBlockType::Unordered; }
    bool HasElementOrBlock(const char* name) const { 
        bool contains = false;

        for (auto it : value_->GetObject())
            if (it.first == name) {
                contains = true;
                break;
            }

        return value_ && contains; 
        // return value_ && value_->GetObject().contains(name);
    }
    void Close(ArchiveBase& archive) {}

private:
    const Value* value_{};

    /// Next array index (for sequential and array blocks).
    unsigned nextElementIndex_{};
};

class JSONInputArchive : public JSONArchiveBase<JSONInputArchiveBlock, true>
{
public:
    /// Base type.
    using Base = JSONArchiveBase<JSONInputArchiveBlock, true>;

    /// Construct from element.
    JSONInputArchive(const Value& value, const JSONFile* jsonFile = nullptr)
        : Base(jsonFile)
        , rootValue_(value)
    {
    }
    /// Construct from file.
    explicit JSONInputArchive(const JSONFile* jsonFile)
        : Base( jsonFile)
        , rootValue_(jsonFile->GetRoot())
    {}

    /// Begin archive block.
    void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) final;

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

private:
    const Value& ReadElement(const char* name);
    void CheckType(const char* name, const Value& value, ValueType type) const;

    const Value& rootValue_;
};

}
