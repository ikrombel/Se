#pragma once

#include <vector>
#include <unordered_map>
#include <utility>

#include <Se/String.hpp>

namespace Se
{

/// JSON value type.
enum JSONValueType
{
    /// JSON null type.
    JSON_NULL = 0,
    /// JSON boolean type.
    JSON_BOOL,
    /// JSON number type.
    JSON_NUMBER,
    /// JSON string type.
    JSON_STRING,
    /// JSON array type.
    JSON_ARRAY,
    /// JSON object type.
    JSON_OBJECT
};

/// JSON number type.
enum JSONNumberType
{
    /// Not a number.
    JSONNT_NAN = 0,
    /// Integer.
    JSONNT_INT,
    /// Unsigned integer.
    JSONNT_UINT,
    /// Float or double.
    JSONNT_FLOAT_DOUBLE
};


class JSONValue;

/// JSON array type.
using JSONArray = std::vector<JSONValue>;
/// JSON object type.
using JSONObject = std::unordered_map<String, JSONValue>;
/// JSON object iterator.
using JSONObjectIterator = JSONObject::iterator;
/// Constant JSON object iterator.
using ConstJSONObjectIterator = JSONObject::const_iterator;

/// JSON value class.
class JSONValue
{

public:
    /// Construct null value.
    JSONValue()
    {
    }

    /// Construct a default value with defined type.
    explicit JSONValue(JSONValueType valueType, JSONNumberType numberType = JSONNT_NAN)
    {
        SetType(valueType, numberType);
    }

    /// Construct with a boolean.
    JSONValue(bool value)        // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a integer.
    JSONValue(int value)         // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a unsigned integer.
    JSONValue(unsigned value)    // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a float.
    JSONValue(float value)       // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a double.
    JSONValue(double value)      // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a string.
    JSONValue(const String& value)   // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a C string.
    JSONValue(const char* value)     // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a JSON array.
    JSONValue(const JSONArray& value)    // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Construct with a JSON object.
    JSONValue(const JSONObject& value)   // NOLINT(google-explicit-constructor)
    {
        *this = value;
    }
    /// Copy-construct from another JSON value.
    JSONValue(const JSONValue& value)
    {
        *this = value;
    }
    /// Move-construct from another JSON value.
    JSONValue(JSONValue && value)
    {
        *this = std::move(value);
    }
    /// Destruct.
    ~JSONValue()
    {
        SetType(JSON_NULL);
    }

    /// Assign from a boolean.
    JSONValue& operator =(bool rhs);
    /// Assign from an integer.
    JSONValue& operator =(int rhs);
    /// Assign from an unsigned integer.
    JSONValue& operator =(unsigned rhs);
    /// Assign from a float.
    JSONValue& operator =(float rhs);
    /// Assign from a double.
    JSONValue& operator =(double rhs);
    /// Assign from a string.
    JSONValue& operator =(const String& rhs);
    /// Assign from a C string.
    JSONValue& operator =(const char* rhs);
    /// Assign from a JSON array.
    JSONValue& operator =(const JSONArray& rhs);
    /// Assign from a JSON object.
    JSONValue& operator =(const JSONObject& rhs);
    /// Assign from another JSON value.
    JSONValue& operator =(const JSONValue& rhs);
    /// Move-assign from another JSON value.
    JSONValue& operator =(JSONValue && rhs);
    /// Value equality operator.
    bool operator ==(const JSONValue& rhs) const;
    /// Value inequality operator.
    bool operator !=(const JSONValue& rhs) const;

    /// Return value type.
    JSONValueType GetValueType() const;
    /// Return number type.
    JSONNumberType GetNumberType() const;
    /// Return value type's name.
    String GetValueTypeName() const;
    /// Return number type's name.
    String GetNumberTypeName() const;

    /// Check is null.
    bool IsNull() const { return GetValueType() == JSON_NULL; }
    /// Check is boolean.
    bool IsBool() const { return GetValueType() == JSON_BOOL; }
    /// Check is number.
    bool IsNumber() const { return GetValueType() == JSON_NUMBER; }
    /// Check is string.
    bool IsString() const { return GetValueType() == JSON_STRING; }
    /// Check is array.
    bool IsArray() const { return GetValueType() == JSON_ARRAY; }
    /// Check is object.
    bool IsObject() const { return GetValueType() == JSON_OBJECT; }

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
    /// Return JSON array value.
    const JSONArray& GetArray() const { return IsArray() ? *arrayValue_ : emptyArray; }
    /// Return JSON object value.
    const JSONObject& GetObject() const { return IsObject() ? *objectValue_ : emptyObject; }

    // JSON array functions
    /// Return JSON value at index.
    JSONValue& operator [](unsigned index);
    /// Return JSON value at index.
    const JSONValue& operator [](unsigned index) const;
    /// Add JSON value at end.
    void Push(const JSONValue& value);
    /// Remove the last JSON value.
    void Pop();
    /// Insert an JSON value at position.
    void Insert(unsigned pos, const JSONValue& value);
    /// Erase a range of JSON values.
    void Erase(unsigned pos, unsigned length = 1);
    /// Resize array.
    void Resize(unsigned newSize);
    /// Return size of array or number of keys in object.
    unsigned Size() const;

    // JSON object functions
    /// Return JSON value with key.
    JSONValue& operator [](const String& key);
    /// Return JSON value with key.
    const JSONValue& operator [](const String& key) const;
    /// Set JSON value with key.
    void Set(const String& key, const JSONValue& value);
    /// Return JSON value with key.
    const JSONValue& Get(const String& key) const;
    /// Return JSON value with index.
    const JSONValue& Get(int index) const;
    /// Erase a pair by key.
    bool Erase(const String& key);
    /// Return whether contains a pair with key.
    bool Contains(const String& key) const;
    /// Return iterator to the beginning.
    JSONObjectIterator begin();
    /// Return iterator to the beginning.
    ConstJSONObjectIterator begin() const;
    /// Return iterator to the end.
    JSONObjectIterator end();
    /// Return iterator to the beginning.
    ConstJSONObjectIterator end() const;

    /// Clear array or object.
    void Clear();

    /// Set value type and number type, internal function.
    void SetType(JSONValueType valueType, JSONNumberType numberType = JSONNT_NAN);

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

    /// Empty JSON value.
    static const JSONValue EMPTY;
    /// Empty JSON array.
    static const JSONArray emptyArray;
    /// Empty JSON object.
    static const JSONObject emptyObject;

    /// Return name corresponding to a value type.
    static String GetValueTypeName(JSONValueType type);
    /// Return name corresponding to a number type.
    static String GetNumberTypeName(JSONNumberType type);
    // /// Return a value type from name; null if unrecognized.
    // static JSONValueType GetValueTypeFromName(const String& typeName);
    // /// Return a value type from name; null if unrecognized.
    // static JSONValueType GetValueTypeFromName(const char* typeName);
    // /// Return a number type from name; NaN if unrecognized.
    // static JSONNumberType GetNumberTypeFromName(const String& typeName);
    // /// Return a value type from name; NaN if unrecognized.
    // static JSONNumberType GetNumberTypeFromName(const char* typeName);

    static bool Compare(const JSONValue& lhs, const JSONValue& rhs);

protected:
    /// type.
    unsigned type_{JSONValueType::JSON_NULL};
    union
    {
        /// Boolean value.
        bool boolValue_;
        /// Number value.
        double numberValue_;
        /// String value.
        String* stringValue_;
        /// Array value.
        JSONArray* arrayValue_;
        /// Object value.
        JSONObject* objectValue_{nullptr};
    };
};

}
