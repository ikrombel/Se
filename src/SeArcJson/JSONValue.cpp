// Copyright (c) 2008-2020 the GFrost project.



#include "JSONValue.h"

#include <cassert>
#include <cmath>
//#include <GFrost/Resource/Serializable.h>

namespace Se
{

static const char* valueTypeNames[] =
{
    "Null",
    "Bool",
    "Number",
    "String",
    "Array",
    "Object",
    nullptr
};

static const char* numberTypeNames[] =
{
    "NaN",
    "Int",
    "Unsigned",
    "Real",
    nullptr
};

const JSONValue JSONValue::EMPTY;
const JSONArray JSONValue::emptyArray { };
const JSONObject JSONValue::emptyObject;

JSONValue& JSONValue::operator =(bool rhs)
{
    SetType(JSON_BOOL);
    boolValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(int rhs)
{
    SetType(JSON_NUMBER, JSONNT_INT);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(unsigned rhs)
{
    SetType(JSON_NUMBER, JSONNT_UINT);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(float rhs)
{
    SetType(JSON_NUMBER, JSONNT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(double rhs)
{
    SetType(JSON_NUMBER, JSONNT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const String& rhs)
{
    SetType(JSON_STRING);
    *stringValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const char* rhs)
{
    SetType(JSON_STRING);
    *stringValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONArray& rhs)
{
    SetType(JSON_ARRAY);
    *arrayValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONObject& rhs)
{
    SetType(JSON_OBJECT);
    *objectValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONValue& rhs)
{
    if (this == &rhs)
        return *this;

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
    case JSON_BOOL:
        boolValue_ = rhs.boolValue_;
        break;

    case JSON_NUMBER:
        numberValue_ = rhs.numberValue_;
        break;

    case JSON_STRING:
        *stringValue_ = *rhs.stringValue_;
        break;

    case JSON_ARRAY:
        *arrayValue_ = *rhs.arrayValue_;
        break;

    case JSON_OBJECT:
        *objectValue_ = *rhs.objectValue_;

    default:
        break;
    }

    return *this;
}

JSONValue& JSONValue::operator=(JSONValue && rhs)
{
    assert(this != &rhs);

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
    case JSON_BOOL:
        boolValue_ = rhs.boolValue_;
        break;

    case JSON_NUMBER:
        numberValue_ = rhs.numberValue_;
        break;

    case JSON_STRING:
        *stringValue_ = std::move(*rhs.stringValue_);
        break;

    case JSON_ARRAY:
        *arrayValue_ = std::move(*rhs.arrayValue_);
        break;

    case JSON_OBJECT:
        *objectValue_ = std::move(*rhs.objectValue_);

    default:
        break;
    }

    return *this;
}

bool JSONValue::operator ==(const JSONValue& rhs) const
{
    // Value type without number type is checked. JSON does not make a distinction between number types. It is possible
    // that we serialized number (for example `1`) as unsigned integer. It will not necessarily be unserialized as same
    // number type. Number value equality check below will make sure numbers match anyway.
    if (GetValueType() != rhs.GetValueType())
        return false;

    switch (GetValueType())
    {
    case JSON_BOOL:
        return boolValue_ == rhs.boolValue_;

    case JSON_NUMBER:
        return numberValue_ == rhs.numberValue_;

    case JSON_STRING:
        return *stringValue_ == *rhs.stringValue_;

    case JSON_ARRAY:
        return *arrayValue_ == *rhs.arrayValue_;

    case JSON_OBJECT:
        return *objectValue_ == *rhs.objectValue_;

    default:
        break;
    }

    return false;
}

bool JSONValue::operator !=(const JSONValue& rhs) const
{
    return !operator ==(rhs);
}

JSONValueType JSONValue::GetValueType() const
{
    return (JSONValueType)(type_ >> 16u);
}

JSONNumberType JSONValue::GetNumberType() const
{
    return (JSONNumberType)(type_ & 0xffffu);
}

String JSONValue::GetValueTypeName() const
{
    return GetValueTypeName(GetValueType());
}

String JSONValue::GetNumberTypeName() const
{
    return GetNumberTypeName(GetNumberType());
}

JSONValue& JSONValue::operator [](unsigned index)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    return (*arrayValue_)[index];
}

const JSONValue& JSONValue::operator [](unsigned index) const
{
    if (GetValueType() != JSON_ARRAY)
        return EMPTY;

    return (*arrayValue_)[index];
}

void JSONValue::Push(const JSONValue& value)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    arrayValue_->push_back(value);
}

void JSONValue::Pop()
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->pop_back();
}

void JSONValue::Insert(unsigned pos, const JSONValue& value)
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->insert(arrayValue_->begin() + pos, value);
}

void JSONValue::Erase(unsigned pos, unsigned length)
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->erase(arrayValue_->begin() + pos, arrayValue_->begin() + pos + length);
}

void JSONValue::Resize(unsigned newSize)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    arrayValue_->resize(newSize);
}

unsigned JSONValue::Size() const
{
    if (GetValueType() == JSON_ARRAY)
        return arrayValue_->size();
    else if (GetValueType() == JSON_OBJECT)
        return objectValue_->size();

    return 0;
}

JSONValue& JSONValue::operator [](const String& key)
{
    // Convert to object type
    SetType(JSON_OBJECT);

    return (*objectValue_)[key];
}

const JSONValue& JSONValue::operator [](const String& key) const
{
    if (GetValueType() != JSON_OBJECT)
        return EMPTY;

    return (*objectValue_)[key];
}

void JSONValue::Set(const String& key, const JSONValue& value)
{
    // Convert to object type
    SetType(JSON_OBJECT);

    (*objectValue_)[key] = value;
}

const JSONValue& JSONValue::Get(int index) const
{
    if (GetValueType() != JSON_ARRAY)
        return EMPTY;

    if (index < 0 || index >= arrayValue_->size())
        return EMPTY;

    return arrayValue_->at(index);
}

const JSONValue& JSONValue::Get(const String& key) const
{
    if (GetValueType() != JSON_OBJECT)
        return EMPTY;

    auto i = objectValue_->find(key);
    if (i == objectValue_->end())
        return EMPTY;

    return i->second;
}

bool JSONValue::Erase(const String& key)
{
    if (GetValueType() != JSON_OBJECT)
        return false;

    return objectValue_->erase(key);
}

bool JSONValue::Contains(const String& key) const
{
    if  (GetValueType() != JSON_OBJECT)
        return false;

    return objectValue_->find(key) != objectValue_->end();
}

JSONObjectIterator JSONValue::begin()
{
    // Convert to object type.
    SetType(JSON_OBJECT);

    return objectValue_->begin();
}

ConstJSONObjectIterator JSONValue::begin() const
{
    if (GetValueType() != JSON_OBJECT)
        return emptyObject.begin();

    return objectValue_->begin();
}

JSONObjectIterator JSONValue::end()
{
    // Convert to object type.
    SetType(JSON_OBJECT);

    return objectValue_->end();
}

ConstJSONObjectIterator JSONValue::end() const
{
    if (GetValueType() != JSON_OBJECT)
        return emptyObject.end();

    return objectValue_->end();
}

void JSONValue::Clear()
{
    if (GetValueType() == JSON_ARRAY)
        arrayValue_->clear();
    else if (GetValueType() == JSON_OBJECT)
        objectValue_->clear();
}

void JSONValue::SetType(JSONValueType valueType, JSONNumberType numberType)
{
    int type = valueType << 16u | numberType;
    if (type == type_)
        return;

    switch (GetValueType())
    {
    case JSON_STRING:
        delete stringValue_;
        break;

    case JSON_ARRAY:
        delete arrayValue_;
        break;

    case JSON_OBJECT:
        delete objectValue_;
        break;

    default:
        break;
    }

    type_ = type;

    switch (GetValueType())
    {
    case JSON_STRING:
        stringValue_ = new String();
        break;

    case JSON_ARRAY:
        arrayValue_ = new JSONArray();
        break;

    case JSON_OBJECT:
        objectValue_ = new JSONObject();
        break;

    default:
        break;
    }
}


String JSONValue::GetValueTypeName(JSONValueType type)
{
    return valueTypeNames[type];
}

String JSONValue::GetNumberTypeName(JSONNumberType type)
{
    return numberTypeNames[type];
}

// JSONValueType JSONValue::GetValueTypeFromName(const std::string& typeName)
// {
//     return GetValueTypeFromName(typeName.c_str());
// }

// JSONValueType JSONValue::GetValueTypeFromName(const char* typeName)
// {
//     return (JSONValueType)GetStringListIndex(typeName, valueTypeNames, JSON_NULL);
// }

// JSONNumberType JSONValue::GetNumberTypeFromName(const std::string& typeName)
// {
//     return GetNumberTypeFromName(typeName.c_str());
// }

// JSONNumberType JSONValue::GetNumberTypeFromName(const char* typeName)
// {
//     return (JSONNumberType)GetStringListIndex(typeName, numberTypeNames, JSONNT_NAN);
// }

bool JSONValue::Compare(const JSONValue& lhs, const JSONValue& rhs) {
    switch (lhs.GetValueType()) {
        case JSONValueType::JSON_STRING:
            return (rhs.IsString() && lhs.GetString() == rhs.GetString());
        case JSONValueType::JSON_NUMBER: {
            if (!rhs.IsNumber())
                return false;
            switch (lhs.GetNumberType()) {
                case JSONNumberType::JSONNT_INT:
                    return ((rhs.GetNumberType() == JSONNumberType::JSONNT_INT)
                            || (rhs.GetNumberType() == JSONNumberType::JSONNT_UINT))
                           && (lhs.GetInt() == rhs.GetInt());
                case JSONNumberType::JSONNT_UINT:
                    return ((rhs.GetNumberType() == JSONNumberType::JSONNT_INT)
                            || (rhs.GetNumberType() == JSONNumberType::JSONNT_UINT))
                           && (lhs.GetUInt() == rhs.GetUInt());
                case JSONNumberType::JSONNT_FLOAT_DOUBLE:
                    return (rhs.GetNumberType() == JSONNumberType::JSONNT_FLOAT_DOUBLE)
                           && (std::lround(lhs.GetDouble()*1000) == std::lround(rhs.GetDouble()*1000));
                case JSONNumberType::JSONNT_NAN:
                    return (rhs.GetNumberType() == JSONNumberType::JSONNT_NAN);
            }
            break;
        }
        case JSONValueType::JSON_BOOL:
            return (rhs.IsBool() && lhs.GetBool() == rhs.GetBool());
        case JSONValueType::JSON_ARRAY:
            if (lhs.GetArray().size() != rhs.GetArray().size())
                return false;
            for (unsigned i = 0; i < lhs.GetArray().size(); i++) {
                if (JSONValue::Compare(lhs.GetArray()[i], rhs.GetArray()[i]))
                    continue;

                return false;
            }
            break;
        case JSONValueType::JSON_OBJECT:

            if (lhs.GetObject().size() != rhs.GetObject().size())
                return false;

            for (auto& it : lhs.GetObject()) {
                if (
                        rhs.Contains(it.first) &&
                        JSONValue::Compare(it.second, rhs[it.first]))
                    continue;

                return false;
            }
            break;
        case JSONValueType::JSON_NULL:
            return rhs.IsNull();
    }

    return true;
}



}
