#pragma once

#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>

#include <Se/String.hpp>
#include <Se/Console.hpp>

namespace Se
{

/// Value value type.
enum ValueType
{
    /// Value null type.
    VALUE_NULL = 0,
    /// Value boolean type.
    VALUE_BOOL,
    /// Value number type.
    VALUE_NUMBER,
    /// Value string type.
    VALUE_STRING,
    /// Value array type.
    VALUE_ARRAY,
    /// Value object type.
    VALUE_OBJECT
};

/// Value number type.
enum ValueNumberType
{
    /// Not a number.
    VALUE_NT_NAN,
    /// Integer.
    VALUE_NT_INT,
    /// Unsigned integer.
    VALUE_NT_UINT,
    /// Float or double.
    VALUE_NT_FLOAT_DOUBLE
};


/// Value value class.
class Value
{
public:
    /// Value array type.
    using Array = std::vector<Value>;
    /// Value object type.
    using Object = std::unordered_map<String, Value>;
    /// Value object iterator.
    using ObjectIterator = Object::iterator;
    /// Constant Value object iterator.
    using ConstObjectIterator = Object::const_iterator;

    /// Construct null value.
    Value()
    {
    }

    /// Construct a default value with defined type.
    explicit Value(ValueType valueType, ValueNumberType numberType = VALUE_NT_NAN)
    {
        SetType(valueType, numberType);
    }

    /// Construct with a boolean.
    Value(bool value)        // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a integer.
    Value(int value)         // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a unsigned integer.
    Value(unsigned value)    // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a float.
    Value(float value)       // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a double.
    Value(double value)      // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a string.
    Value(const String& value)   // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a C string.
    Value(const char* value)     // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a Value array.
    Value(const Array& value)    // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a Value object.
    Value(const Object& value)   // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Copy-construct from another Value value.
    Value(const Value& value)
    {
        *this = value;
    }
    /// Move-construct from another Value value.
    Value(Value && value)
    {
        *this = std::move(value);
    }
    /// Destruct.
    ~Value()
    {
        SetType(VALUE_NULL);
    }

    /// Assign from a boolean.
    Value& operator =(bool rhs);
    /// Assign from an integer.
    Value& operator =(int rhs);
    /// Assign from an unsigned integer.
    Value& operator =(unsigned rhs);
    /// Assign from a float.
    Value& operator =(float rhs);
    /// Assign from a double.
    Value& operator =(double rhs);
    /// Assign from a string.
    Value& operator =(const String& rhs);
    /// Assign from a C string.
    Value& operator =(const char* rhs);
    /// Assign from a Value array.
    Value& operator =(const Array& rhs);
    /// Assign from a Value object.
    Value& operator =(const Object& rhs);
    /// Assign from another Value value.
    Value& operator =(const Value& rhs);
    /// Move-assign from another Value value.
    Value& operator =(Value && rhs);
    /// Value equality operator.
    bool operator ==(const Value& rhs) const;
    /// Value inequality operator.
    bool operator !=(const Value& rhs) const;

    /// Return value type.
    ValueType GetValueType() const;
    /// Return number type.
    ValueNumberType GetNumberType() const;
    /// Return value type's name.
    String GetValueTypeName() const;
    /// Return number type's name.
    String GetNumberTypeName() const;

    /// Check is null.
    bool IsNull() const { return GetValueType() == VALUE_NULL; }
    /// Check is boolean.
    bool IsBool() const { return GetValueType() == VALUE_BOOL; }
    /// Check is number.
    bool IsNumber() const { return GetValueType() == VALUE_NUMBER; }
    /// Check is string.
    bool IsString() const { return GetValueType() == VALUE_STRING; }
    /// Check is array.
    bool IsArray() const { return GetValueType() == VALUE_ARRAY; }
    /// Check is object.
    bool IsObject() const { return GetValueType() == VALUE_OBJECT; }

    /// Return boolean value.
    bool GetBool(bool defaultValue = false) const { return IsBool() ? boolValue_ : defaultValue;}
    /// Return integer value.
    int GetInt(int defaultValue = 0) const { return IsNumber() ? (int)numberValue_ : defaultValue; }
    /// Return unsigned integer value.
    unsigned GetUInt(unsigned defaultValue = 0) const { return IsNumber() ? (unsigned)numberValue_ : defaultValue; }
    /// Return float value.
    float GetFloat(float defaultValue = 0.0f) const { return IsNumber() ? (float)numberValue_ : defaultValue; }
    /// Return double value.
    double GetDouble(double defaultValue = 0.0) const { return IsNumber() ? numberValue_ : defaultValue; }
    /// Return string value. The 'defaultValue' may potentially be returned as is, so it is the responsibility of the caller to ensure the 'defaultValue' remains valid while the return value is being referenced.
    const String& GetString(const String& defaultValue = "") const { return IsString() ? *stringValue_ : defaultValue;}
    /// Return C string value. Default to empty string literal.
    const char* GetCString(const char* defaultValue = "") const { return IsString() ? stringValue_->c_str() : defaultValue;}
    /// Return Value array value.
    const Array& GetArray() const { return IsArray() ? *arrayValue_ : emptyArray; }
    /// Return Value object value.
    const Object& GetObject() const { return IsObject() ? *objectValue_ : emptyObject; }

    // Value array functions
    /// Return Value value at index.
    Value& operator [](unsigned index);
    /// Return Value value at index.
    const Value& operator [](unsigned index) const;
    /// Add Value value at end.
    void Push(const Value& value);
    /// Remove the last Value value.
    void Pop();
    /// Insert an Value value at position.
    void Insert(unsigned pos, const Value& value);
    /// Erase a range of Value values.
    void Erase(unsigned pos, unsigned length = 1);
    /// Resize array.
    void Resize(unsigned newSize);
    /// Return size of array or number of keys in object.
    unsigned Size() const;

    // Value object functions
    /// Return Value value with key.
    Value& operator [](const String& key);
    /// Return Value value with key.
    const Value& operator [](const String& key) const;
    /// Set Value value with key.
    void Set(const String& key, const Value& value);
    /// Return Value value with key.
    const Value& Get(const String& key) const;
    /// Return Value value with index.
    const Value& Get(int index) const;
    /// Erase a pair by key.
    bool Erase(const String& key);
    /// Return whether contains a pair with key.
    bool Contains(const String& key) const;
    /// Return iterator to the beginning.
    ObjectIterator begin();
    /// Return iterator to the beginning.
    ConstObjectIterator begin() const;
    /// Return iterator to the end.
    ObjectIterator end();
    /// Return iterator to the beginning.
    ConstObjectIterator end() const;

    /// Clear array or object.
    void Clear();

    /// Set value type and number type, internal function.
    void SetType(ValueType valueType, ValueNumberType numberType = VALUE_NT_NAN);

    // /// Set variant, context must provide for resource ref.
    // void SetVariant(const Variant& variant, Context* context = nullptr);
    // /// Return a variant.
    // Variant GetVariant() const;
    // /// Set variant value, context must provide for resource ref.
    // void SetVariantValue(const Variant& variant, Context* context = nullptr);
    // /// Return a variant with type.
    // Variant GetVariantValue(VariantType type, Context* context = nullptr) const;
    // /// Set variant map, context must provide for resource ref.
    // void SetVariantMap(const VariantMap& variantMap, Context* context = nullptr);
    // /// Return a variant map.
    // VariantMap GetVariantMap() const;
    // /// Set variant vector, context must provide for resource ref.
    // void SetVariantVector(const VariantVector& variantVector, Context* context = nullptr);
    // /// Return a variant vector.
    // VariantVector GetVariantVector() const;
    // /// Set string variant map, context must provide for resource ref.
    // void SetStringVariantMap(const StringVariantMap& variantMap, Context* context = nullptr);
    // /// Return a string variant map.
    // StringVariantMap GetStringVariantMap() const;

    /// Empty Value value.
    static const Value EMPTY;
    /// Empty Value array.
    static const Array emptyArray;
    /// Empty Value object.
    static const Object emptyObject;

    /// Return name corresponding to a value type.
    static String GetValueTypeName(ValueType type);
    /// Return name corresponding to a number type.
    static String GetNumberTypeName(ValueNumberType type);
    // /// Return a value type from name; null if unrecognized.
    // static ValueType GetValueTypeFromName(const String& typeName);
    // /// Return a value type from name; null if unrecognized.
    // static ValueType GetValueTypeFromName(const char* typeName);
    // /// Return a number type from name; NaN if unrecognized.
    // static ValueNumberType GetNumberTypeFromName(const String& typeName);
    // /// Return a value type from name; NaN if unrecognized.
    // static ValueNumberType GetNumberTypeFromName(const char* typeName);

    static bool Compare(const Value& lhs, const Value& rhs);

    template <typename T>
    void RegisterObjectValue(const String& key, std::function<void(T, Value&)> serializer, std::function<T(Value)> deserializer)
    {
        objectValueMap_.emplace(key, std::make_pair(serializer, deserializer));
    }

    template <typename T>
    void RegisterObjectValue(std::function<void(Value&, const T&)> serializer, std::function<void(const Value&, T&)> deserializer)
    {
        String key = typeid(T).name();

        if (objectValueMap_.find(key) != objectValueMap_.end())
        {
            SE_LOG_ERROR("Object value with key '{}' already registered.", key);
            return;
        }

        ObjectValueSerializer ovs;
        ovs.serializer = [serializer](Value& v, const void* val) {
            serializer(v, *static_cast<const T*>(val));
        };
        ovs.deserializer = [deserializer](const Value& v, void* val) {
            deserializer(v, *static_cast<T*>(val));
        };

        objectValueMap_.emplace(std::move(key), ovs);
    }

    /// Return Value with key.
    template <typename T>
    const T& Get(const String& key) const
    {
        T ret{};

        String typeStr = typeid(T).name();

        auto it = objectValueMap_.find(typeStr);
        if (it != objectValueMap_.end())
        {
            it->second.deserializer((*this)[key], (void*)&ret);
            return ret;
        }
        SE_LOG_ERROR("Object value {} with key '{}' not found.", typeStr, key);
        return ret;
    }

    /// Set Value value with key.
    template <typename T>
    void Set(const String& key, T value)
    {
        String typeStr = typeid(T).name();

        auto it = objectValueMap_.find(typeStr);
        if (it == objectValueMap_.end())
        {
             SE_LOG_ERROR("Object value {} with key '{}' not found.", typeStr, key);
        }

        it->second.serializer((*this)[key], (const void*)&value);
    }

protected:
    /// type.
    unsigned type_{ValueType::VALUE_NULL};
    union
    {
        /// Boolean value.
        bool boolValue_;
        /// Number value.
        double numberValue_;
        /// String value.
        String* stringValue_;
        /// Array value.
        Array* arrayValue_;
        /// Object value.
        Object* objectValue_{nullptr};
    };

    struct ObjectValueSerializer
    {
        std::function<void(Value&, const void*)> serializer;
        std::function<void(const Value&, void*)> deserializer;
    };

    inline static std::unordered_map<String, ObjectValueSerializer> objectValueMap_;
};

}
