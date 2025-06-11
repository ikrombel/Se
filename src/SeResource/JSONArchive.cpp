// Copyright (c) 2017-2020 the rbfx project.

//#include <GFrost/Core/StringUtils.h>
#include <SeArc/ArchiveSerialization.hpp>
#include <SeResource/JSONArchive.h>

#include <string>

namespace Se
{

namespace {

inline bool IsArchiveBlockValueArray(ArchiveBlockType type) {
    return type == ArchiveBlockType::Array || type == ArchiveBlockType::Sequential;
}

inline bool IsArchiveBlockValueObject(ArchiveBlockType type) {
    return type == ArchiveBlockType::Unordered;
}

inline bool IsValueCompatibleWithArray(const Value& value) {
    return value.IsArray() || value.IsNull() || (value.IsObject() && value.GetObject().empty());
}

inline bool IsValueCompatibleWithObject(const Value& value) {
    return value.IsObject() || value.IsNull() || (value.IsArray() && value.GetArray().empty());
}

inline bool IsArchiveBlockTypeMatching(const JSONValue& value, ArchiveBlockType type) {
    return (IsArchiveBlockValueArray(type) && IsValueCompatibleWithArray(value))
           || (IsArchiveBlockValueObject(type) && IsValueCompatibleWithObject(value));
}

}

JSONOutputArchiveBlock::JSONOutputArchiveBlock(const char* name, ArchiveBlockType type, JSONValue* blockValue, unsigned sizeHint)
        : ArchiveBlockBase(name, type)
        , blockValue_(blockValue)
{
    if (type_ == ArchiveBlockType::Array)
        expectedElementCount_ = sizeHint;

    if (IsArchiveBlockValueArray(type_))
        blockValue_->SetType(VALUE_ARRAY);
    else if (IsArchiveBlockValueObject(type_))
        blockValue_->SetType(VALUE_OBJECT);
    else
        assert(0);
}

    JSONValue& JSONOutputArchiveBlock::CreateElement(ArchiveBase& archive, const char* elementName)
    {
        assert(numElements_ < expectedElementCount_);

        switch (type_)
        {
            case ArchiveBlockType::Sequential:
            case ArchiveBlockType::Array:
                ++numElements_;
                blockValue_->Push(JSONValue{});
                return (*blockValue_)[blockValue_->Size() - 1];

            case ArchiveBlockType::Unordered: {
                auto it = blockValue_->GetObject().find(elementName);

                if (it != blockValue_->GetObject().end())
                    throw archive.DuplicateElementException(elementName);

                ++numElements_;
                blockValue_->Set(elementName, JSONValue{});
                return (*blockValue_)[elementName];
            }
            default:
                assert(0);
                return *blockValue_;
        }
    }

    void JSONOutputArchiveBlock::Close(ArchiveBase& archive)
    {
        // TODO: Uncomment when PluginManager is fixed
        //SE_ASSERT(expectedElementCount_ == M_MAX_UNSIGNED || numElements_ == expectedElementCount_);
    }

    void JSONOutputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
    {
        CheckBeforeBlock(name);
        CheckBlockOrElementName(name);

        // Open root block
        if (stack_.empty())
        {
            stack_.push_back(Block{ name, type, &rootValue_, sizeHint });
            return;
        }

        // Validate new block
        JSONValue& blockValue = GetCurrentBlock().CreateElement(*this, name);
        stack_.push_back(Block{ name, type, &blockValue, sizeHint });
    }

    void JSONOutputArchive::Serialize(const char* name, long long& value)
    {
        CreateElement(name, JSONValue(String(std::to_string(value).c_str()) ));
    }

    void JSONOutputArchive::Serialize(const char* name, unsigned long long& value)
    {
        CreateElement(name, JSONValue{ format("{}", value).c_str() });
    }

    void JSONOutputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
    {
        BufferToHexString(tempString_, bytes, size);
        CreateElement(name, JSONValue{ tempString_ });
    }

    void JSONOutputArchive::SerializeVLE(const char* name, unsigned& value)
    {
        CreateElement(name, JSONValue{ value });
    }

    void JSONOutputArchive::CreateElement(const char* name, const JSONValue& value)
    {
        CheckBeforeElement(name);
        CheckBlockOrElementName(name);

        JSONValue& jsonValue = GetCurrentBlock().CreateElement(*this, name);
        jsonValue = value;
    }

// Generate serialization implementation (JSON output)
#define SE_VALUE_OUT_IMPL(type, function) \
    void JSONOutputArchive::Serialize(const char* name, type& value) \
    { \
        CreateElement(name, JSONValue{ value }); \
    }

    SE_VALUE_OUT_IMPL(bool, SetBool);
    SE_VALUE_OUT_IMPL(signed char, SetInt);
    SE_VALUE_OUT_IMPL(short, SetInt);
    SE_VALUE_OUT_IMPL(int, SetInt);
    SE_VALUE_OUT_IMPL(unsigned char, SetUInt);
    SE_VALUE_OUT_IMPL(unsigned short, SetUInt);
    SE_VALUE_OUT_IMPL(unsigned int, SetUInt);
    SE_VALUE_OUT_IMPL(float, SetFloat);
    SE_VALUE_OUT_IMPL(double, SetDouble);
    SE_VALUE_OUT_IMPL(String, SetString);

#undef SE_VALUE_OUT_IMPL

    JSONInputArchiveBlock::JSONInputArchiveBlock(const char* name, ArchiveBlockType type, const JSONValue* value)
            : ArchiveBlockBase(name, type)
            , value_(value)
    {
    }

    const JSONValue& JSONInputArchiveBlock::ReadElement(ArchiveBase& archive, const char* elementName, const ArchiveBlockType* elementBlockType)
    {
        // Find appropriate value
        const JSONValue* elementValue = nullptr;
        if (IsArchiveBlockValueArray(type_))
        {
            if (nextElementIndex_ >= value_->Size())
                throw archive.ElementNotFoundException(elementName, nextElementIndex_);

            elementValue = &value_->Get(nextElementIndex_);
            ++nextElementIndex_;
        }
        else if (IsArchiveBlockValueObject(type_))
        {
            if (!value_->Contains(elementName))
                throw archive.ElementNotFoundException(elementName);

            elementValue = &value_->Get(elementName);
        }
        else
        {
            assert(0);
            return *value_;
        }

        // Check if reading block
        if (elementBlockType && !IsArchiveBlockTypeMatching(*elementValue, *elementBlockType))
            throw archive.UnexpectedElementValueException(name_);

        return *elementValue;
    }

    void JSONInputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
    {
        CheckBeforeBlock(name);
        CheckBlockOrElementName(name);

        // Open root block
        if (stack_.empty())
        {
            if (!IsArchiveBlockTypeMatching(rootValue_, type))
                throw UnexpectedElementValueException(name);

            Block frame{ name, type, &rootValue_ };
            sizeHint = frame.GetSizeHint();
            stack_.push_back(frame);
            return;
        }

        // Open block
        const JSONValue& blockValue = GetCurrentBlock().ReadElement(*this, name, &type);

        Block blockFrame{ name, type, &blockValue };
        sizeHint = blockFrame.GetSizeHint();
        stack_.push_back(blockFrame);
    }

    void JSONInputArchive::Serialize(const char* name, long long& value)
    {
        const JSONValue& jsonValue = ReadElement(name);
        CheckType(name, jsonValue, VALUE_STRING);

        const String stringValue = jsonValue.GetString();
        

        value = ToInt64(stringValue.c_str());
    }

    void JSONInputArchive::Serialize(const char* name, unsigned long long& value)
    {
        const JSONValue& jsonValue = ReadElement(name);
        CheckType(name, jsonValue, VALUE_STRING);

        const String stringValue = jsonValue.GetString();
        value = ToUInt64(stringValue.c_str());
    }

    void JSONInputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
    {
        const JSONValue& jsonValue = ReadElement(name);
        CheckType(name, jsonValue, VALUE_STRING);

        ReadBytesFromHexString(name, jsonValue.GetString(), bytes, size);
    }

    void JSONInputArchive::SerializeVLE(const char* name, unsigned& value)
    {
        const JSONValue& jsonValue = ReadElement(name);
        CheckType(name, jsonValue, VALUE_NUMBER);

        value = jsonValue.GetUInt();
    }

    const JSONValue& JSONInputArchive::ReadElement(const char* name)
    {
        CheckBeforeElement(name);
        CheckBlockOrElementName(name);
        return GetCurrentBlock().ReadElement(*this, name, nullptr);
    }

    void JSONInputArchive::CheckType(const char* name, const Value& value, ValueType type) const
    {
        if (value.GetValueType() != type)
            throw UnexpectedElementValueException(name);
    }

// Generate serialization implementation (JSON input)
#define SE_VALUE_IN_IMPL(type, function, jsonType) \
    void JSONInputArchive::Serialize(const char* name, type& value) \
    { \
        const Value& jsonValue = ReadElement(name); \
        CheckType(name, jsonValue, jsonType); \
        value = jsonValue.function(); \
    }

    SE_VALUE_IN_IMPL(bool, GetBool, VALUE_BOOL);
    SE_VALUE_IN_IMPL(signed char, GetInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(short, GetInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(int, GetInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(unsigned char, GetUInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(unsigned short, GetUInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(unsigned int, GetUInt, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(float, GetFloat, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(double, GetDouble, VALUE_NUMBER);
    SE_VALUE_IN_IMPL(String, GetString, VALUE_STRING);

#undef SE_VALUE_IN_IMPL

}
