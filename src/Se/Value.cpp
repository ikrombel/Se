#include "Value.h"

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

const Value Value::EMPTY;
const Value::Array Value::emptyArray { };
const Value::Object Value::emptyObject;

Value& Value::operator =(bool rhs)
{
    SetType(VALUE_BOOL);
    boolValue_ = rhs;

    return *this;
}

Value& Value::operator =(int rhs)
{
    SetType(VALUE_NUMBER, VALUE_NT_INT);
    numberValue_ = rhs;

    return *this;
}

Value& Value::operator =(unsigned rhs)
{
    SetType(VALUE_NUMBER, VALUE_NT_UINT);
    numberValue_ = rhs;

    return *this;
}

Value& Value::operator =(float rhs)
{
    SetType(VALUE_NUMBER, VALUE_NT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

Value& Value::operator =(double rhs)
{
    SetType(VALUE_NUMBER, VALUE_NT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

Value& Value::operator =(const String& rhs)
{
    SetType(VALUE_STRING);
    *stringValue_ = rhs;

    return *this;
}

Value& Value::operator =(const char* rhs)
{
    SetType(VALUE_STRING);
    *stringValue_ = rhs;

    return *this;
}

Value& Value::operator =(const Array& rhs)
{
    SetType(VALUE_ARRAY);
    *arrayValue_ = rhs;

    return *this;
}

Value& Value::operator =(const Object& rhs)
{
    SetType(VALUE_OBJECT);
    *objectValue_ = rhs;

    return *this;
}

Value& Value::operator =(const Value& rhs)
{
    if (this == &rhs)
        return *this;

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
    case VALUE_BOOL:
        boolValue_ = rhs.boolValue_;
        break;

    case VALUE_NUMBER:
        numberValue_ = rhs.numberValue_;
        break;

    case VALUE_STRING:
        *stringValue_ = *rhs.stringValue_;
        break;

    case VALUE_ARRAY:
        *arrayValue_ = *rhs.arrayValue_;
        break;

    case VALUE_OBJECT:
        *objectValue_ = *rhs.objectValue_;

    default:
        break;
    }

    return *this;
}

Value& Value::operator=(Value && rhs)
{
    assert(this != &rhs);

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
    case VALUE_BOOL:
        boolValue_ = rhs.boolValue_;
        break;

    case VALUE_NUMBER:
        numberValue_ = rhs.numberValue_;
        break;

    case VALUE_STRING:
        *stringValue_ = std::move(*rhs.stringValue_);
        break;

    case VALUE_ARRAY:
        *arrayValue_ = std::move(*rhs.arrayValue_);
        break;

    case VALUE_OBJECT:
        *objectValue_ = std::move(*rhs.objectValue_);

    default:
        break;
    }

    return *this;
}

bool Value::operator ==(const Value& rhs) const
{
    // Value type without number type is checked. JSON does not make a distinction between number types. It is possible
    // that we serialized number (for example `1`) as unsigned integer. It will not necessarily be unserialized as same
    // number type. Number value equality check below will make sure numbers match anyway.
    if (GetValueType() != rhs.GetValueType())
        return false;

    switch (GetValueType())
    {
    case VALUE_BOOL:
        return boolValue_ == rhs.boolValue_;

    case VALUE_NUMBER:
        return numberValue_ == rhs.numberValue_;

    case VALUE_STRING:
        return *stringValue_ == *rhs.stringValue_;

    case VALUE_ARRAY:
        return *arrayValue_ == *rhs.arrayValue_;

    case VALUE_OBJECT:
        return *objectValue_ == *rhs.objectValue_;

    default:
        break;
    }

    return false;
}

bool Value::operator !=(const Value& rhs) const
{
    return !operator ==(rhs);
}

ValueType Value::GetValueType() const
{
    return (ValueType)(type_ >> 16u);
}

ValueNumberType Value::GetNumberType() const
{
    return (ValueNumberType)(type_ & 0xffffu);
}

String Value::GetValueTypeName() const
{
    return GetValueTypeName(GetValueType());
}

String Value::GetNumberTypeName() const
{
    return GetNumberTypeName(GetNumberType());
}

Value& Value::operator [](unsigned index)
{
    // Convert to array type
    SetType(VALUE_ARRAY);

    return (*arrayValue_)[index];
}

const Value& Value::operator [](unsigned index) const
{
    if (GetValueType() != VALUE_ARRAY)
        return EMPTY;

    return (*arrayValue_)[index];
}

void Value::Push(const Value& value)
{
    // Convert to array type
    SetType(VALUE_ARRAY);

    arrayValue_->push_back(value);
}

void Value::Pop()
{
    if (GetValueType() != VALUE_ARRAY)
        return;

    arrayValue_->pop_back();
}

void Value::Insert(unsigned pos, const Value& value)
{
    if (GetValueType() != VALUE_ARRAY)
        return;

    arrayValue_->insert(arrayValue_->begin() + pos, value);
}

void Value::Erase(unsigned pos, unsigned length)
{
    if (GetValueType() != VALUE_ARRAY)
        return;

    arrayValue_->erase(arrayValue_->begin() + pos, arrayValue_->begin() + pos + length);
}

void Value::Resize(unsigned newSize)
{
    // Convert to array type
    SetType(VALUE_ARRAY);

    arrayValue_->resize(newSize);
}

unsigned Value::Size() const
{
    if (GetValueType() == VALUE_ARRAY)
        return arrayValue_->size();
    else if (GetValueType() == VALUE_OBJECT)
        return objectValue_->size();

    return 0;
}

Value& Value::operator [](const String& key)
{
    // Convert to object type
    SetType(VALUE_OBJECT);

    return (*objectValue_)[key];
}

const Value& Value::operator [](const String& key) const
{
    if (GetValueType() != VALUE_OBJECT)
        return EMPTY;

    return (*objectValue_)[key];
}

void Value::Set(const String& key, const Value& value)
{
    // Convert to object type
    SetType(VALUE_OBJECT);

    (*objectValue_)[key] = value;
}

const Value& Value::Get(int index) const
{
    if (GetValueType() != VALUE_ARRAY)
        return EMPTY;

    if (index < 0 || index >= arrayValue_->size())
        return EMPTY;

    return arrayValue_->at(index);
}

const Value& Value::Get(const String& key) const
{
    if (GetValueType() != VALUE_OBJECT)
        return EMPTY;

    auto i = objectValue_->find(key);
    if (i == objectValue_->end())
        return EMPTY;

    return i->second;
}

bool Value::Erase(const String& key)
{
    if (GetValueType() != VALUE_OBJECT)
        return false;

    return objectValue_->erase(key);
}

bool Value::Contains(const String& key) const
{
    if  (GetValueType() != VALUE_OBJECT)
        return false;

    return objectValue_->find(key) != objectValue_->end();
}

Value::ObjectIterator Value::begin()
{
    // Convert to object type.
    SetType(VALUE_OBJECT);

    return objectValue_->begin();
}

Value::ConstObjectIterator Value::begin() const
{
    if (GetValueType() != VALUE_OBJECT)
        return emptyObject.begin();

    return objectValue_->begin();
}

Value::ObjectIterator Value::end()
{
    // Convert to object type.
    SetType(VALUE_OBJECT);

    return objectValue_->end();
}

Value::ConstObjectIterator Value::end() const
{
    if (GetValueType() != VALUE_OBJECT)
        return emptyObject.end();

    return objectValue_->end();
}

void Value::Clear()
{
    if (GetValueType() == VALUE_ARRAY)
        arrayValue_->clear();
    else if (GetValueType() == VALUE_OBJECT)
        objectValue_->clear();
}

void Value::SetType(ValueType valueType, ValueNumberType numberType)
{
    int type = valueType << 16u | numberType;
    if (type == type_)
        return;

    switch (GetValueType())
    {
    case VALUE_STRING:
        delete stringValue_;
        break;

    case VALUE_ARRAY:
        delete arrayValue_;
        break;

    case VALUE_OBJECT:
        delete objectValue_;
        break;

    default:
        break;
    }

    type_ = type;

    switch (GetValueType())
    {
    case VALUE_STRING:
        stringValue_ = new String();
        break;

    case VALUE_ARRAY:
        arrayValue_ = new Array();
        break;

    case VALUE_OBJECT:
        objectValue_ = new Object();
        break;

    default:
        break;
    }
}


String Value::GetValueTypeName(ValueType type)
{
    return valueTypeNames[type];
}

String Value::GetNumberTypeName(ValueNumberType type)
{
    return numberTypeNames[type];
}

// ValueType Value::GetValueTypeFromName(const std::string& typeName)
// {
//     return GetValueTypeFromName(typeName.c_str());
// }

// ValueType Value::GetValueTypeFromName(const char* typeName)
// {
//     return (ValueType)GetStringListIndex(typeName, valueTypeNames, VALUE_NULL);
// }

// ValueNumberType Value::GetNumberTypeFromName(const std::string& typeName)
// {
//     return GetNumberTypeFromName(typeName.c_str());
// }

// ValueNumberType Value::GetNumberTypeFromName(const char* typeName)
// {
//     return (ValueNumberType)GetStringListIndex(typeName, numberTypeNames, VALUE_NT_NAN);
// }

bool Value::Compare(const Value& lhs, const Value& rhs) {
    switch (lhs.GetValueType()) {
        case ValueType::VALUE_STRING:
            return (rhs.IsString() && lhs.GetString() == rhs.GetString());
        case ValueType::VALUE_NUMBER: {
            if (!rhs.IsNumber())
                return false;
            switch (lhs.GetNumberType()) {
                case ValueNumberType::VALUE_NT_INT:
                    return ((rhs.GetNumberType() == ValueNumberType::VALUE_NT_INT)
                            || (rhs.GetNumberType() == ValueNumberType::VALUE_NT_UINT))
                           && (lhs.GetInt() == rhs.GetInt());
                case ValueNumberType::VALUE_NT_UINT:
                    return ((rhs.GetNumberType() == ValueNumberType::VALUE_NT_INT)
                            || (rhs.GetNumberType() == ValueNumberType::VALUE_NT_UINT))
                           && (lhs.GetUInt() == rhs.GetUInt());
                case ValueNumberType::VALUE_NT_FLOAT_DOUBLE:
                    return (rhs.GetNumberType() == ValueNumberType::VALUE_NT_FLOAT_DOUBLE)
                           && (std::lround(lhs.GetDouble()*1000) == std::lround(rhs.GetDouble()*1000));
                case ValueNumberType::VALUE_NT_NAN:
                    return (rhs.GetNumberType() == ValueNumberType::VALUE_NT_NAN);
            }
            break;
        }
        case ValueType::VALUE_BOOL:
            return (rhs.IsBool() && lhs.GetBool() == rhs.GetBool());
        case ValueType::VALUE_ARRAY:
            if (lhs.GetArray().size() != rhs.GetArray().size())
                return false;
            for (unsigned i = 0; i < lhs.GetArray().size(); i++) {
                if (Value::Compare(lhs.GetArray()[i], rhs.GetArray()[i]))
                    continue;

                return false;
            }
            break;
        case ValueType::VALUE_OBJECT:

            if (lhs.GetObject().size() != rhs.GetObject().size())
                return false;

            for (auto& it : lhs.GetObject()) {
                if (rhs.Contains(it.first) && Value::Compare(it.second, rhs[it.first]))
                    continue;

                return false;
            }
            break;
        case ValueType::VALUE_NULL:
            return rhs.IsNull();
    }

    return true;
}



}
