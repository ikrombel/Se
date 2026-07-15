// Copyright (c) 2008-2020 the GFrost project.

#pragma once


#include <Se/Export.hpp>
#include <SeMath/BoundingBox.hpp>
#include <SeMath/Rect.hpp>
#include <SeMath/Color.hpp>

#if __has_include(<SeEngine/Core/Variant.h>)
#include <SeEngine/Core/Variant.h>
#endif

#include <memory>



namespace pugi
{

struct xml_node_struct;
class xpath_node;
class xpath_node_set;
class xpath_query;
class xpath_variable_set;

}

namespace Se
{

class XMLFile;
class XPathQuery;
class XPathResultSet;

/// Element in an XML file.
class SE_API XMLElement
{
public:
    /// Construct null element.
    XMLElement();
    /// Construct with document and node pointers.
    XMLElement(XMLFile* file, pugi::xml_node_struct* node);
    /// Construct from xpath query result set.
    XMLElement(XMLFile* file, const XPathResultSet* resultSet, const pugi::xpath_node* xpathNode, unsigned xpathResultIndex);
    /// Copy-construct from another element.
    XMLElement(const XMLElement& rhs);
    /// Destruct.
    virtual ~XMLElement();
    /// Assignment operator.
    XMLElement& operator =(const XMLElement& rhs);

    /// Set element name.
    void SetName(const String& name);
    /// Set element name.
    void SetName(const char* name);

    /// Create a child element.
    //XMLElement CreateChild(const String& name);
    /// Create a child element.
    XMLElement CreateChild(const char* name);
    /// Return the first child element with name or create if does not exist.
    XMLElement GetOrCreateChild(const String& name);
    /// Return the first child element with name or create if does not exist.
    XMLElement GetOrCreateChild(const char* name);
    /// Append element. If asCopy is set to true then original element is copied and appended, otherwise specified element is appended.
    bool AppendChild(XMLElement element, bool asCopy = false);
    /// Remove element from its parent.
    bool Remove();
    /// Remove a child element. Return true if successful.
    bool RemoveChild(const XMLElement& element);
    /// Remove a child element by name. Return true if successful.
    bool RemoveChild(const String& name);
    /// Remove a child element by name. Return true if successful.
    bool RemoveChild(const char* name);
    /// Remove child elements of certain name, or all child elements if name is empty. Return true if successful.
    bool RemoveChildren(const String& name = String::EMPTY);
    /// Remove child elements of certain name, or all child elements if name is empty. Return true if successful.
    bool RemoveChildren(const char* name);
    /// Remove an attribute by name. Return true if successful.
    bool RemoveAttribute(const String& name = String::EMPTY);
    /// Remove an attribute by name. Return true if successful.
    bool RemoveAttribute(const char* name);

    /// Select an element/attribute using XPath query.
    XMLElement SelectSingle(const String& query, pugi::xpath_variable_set* variables = nullptr) const;
    /// Select an element/attribute using XPath query.
    XMLElement SelectSinglePrepared(const XPathQuery& query) const;
    /// Select elements/attributes using XPath query.
    XPathResultSet Select(const String& query, pugi::xpath_variable_set* variables = nullptr) const;
    /// Select elements/attributes using XPath query.
    XPathResultSet SelectPrepared(const XPathQuery& query) const;

    /// Set the value for an inner node in the following format <node>value</node>.
    bool SetValue(const String& value);
    /// Set the value for an inner node in the following format <node>value</node>. Must be used on the <node> element.
    bool SetValue(const char* value);
    /// Set an attribute.
    bool SetAttribute(const String& name, const String& value);
    /// Set an attribute.
    bool SetAttribute(const char* name, const char* value);
    /// Set an attribute. Only valid if it is an attribute only XPath query result.
    bool SetAttribute(const String& value);
    /// Set an attribute. Only valid if it is an attribute only XPath query result.
    bool SetAttribute(const char* value);
    /// Set a bool attribute.
    bool SetBool(const String& name, bool value);
    /// Set a BoundingBox attribute.
    bool SetBoundingBox(const BoundingBox& value);
    // /// Set a buffer attribute.
    // bool SetBuffer(const String& name, const void* data, unsigned size);
    // /// Set a buffer attribute.
    // bool SetBuffer(const String& name, const std::vector<unsigned char>& value);
    /// Set a color attribute.
    bool SetColor(const String& name, const Color& value);
    /// Set a float attribute.
    bool SetFloat(const String& name, float value);
    /// Set a double attribute.
    bool SetDouble(const String& name, double value);
    /// Set an unsigned integer attribute.
    bool SetUInt(const String& name, unsigned value);
    /// Set an integer attribute.
    bool SetInt(const String& name, int value);
    /// Set an unsigned long long integer attribute.
    bool SetUInt64(const String& name, unsigned long long value);
    /// Set a long long integer attribute.
    bool SetInt64(const String& name, long long value);
    /// Set an IntRect attribute.
    bool SetIntRect(const String& name, const IntRect& value);
    /// Set an IntVector2 attribute.
    bool SetIntVector2(const String& name, const IntVector2& value);
    /// Set an IntVector3 attribute.
    bool SetIntVector3(const String& name, const IntVector3& value);
    /// Set a Rect attribute.
    bool SetRect(const String& name, const Rect& value);
    /// Set a quaternion attribute.
    bool SetQuaternion(const String& name, const Quaternion& value);
    /// Set a string attribute.
    bool SetString(const String& name, const String& value);
    // /// Set a variant attribute.
    // bool SetVariant(const Variant& value);
    // /// Set a variant attribute excluding the type.
    // bool SetVariantValue(const Variant& value);
    // /// Set a resource reference attribute.
    // bool SetResourceRef(const ResourceRef& value);
    // /// Set a resource reference list attribute.
    // bool SetResourceRefList(const ResourceRefList& value);
    // /// Set a variant vector attribute. Creates child elements as necessary.
    // bool SetVariantVector(const VariantVector& value);
    /// Set a string vector attribute. Creates child elements as necessary.
    bool SetStringVector(const std::vector<String>& value);
    // /// Set a variant map attribute. Creates child elements as necessary.
    // bool SetVariantMap(const VariantMap& value);
    /// Set a Vector2 attribute.
    bool SetVector2(const String& name, const Vector2& value);
    /// Set a Vector3 attribute.
    bool SetVector3(const String& name, const Vector3& value);
    /// Set a Vector4 attribute.
    bool SetVector4(const String& name, const Vector4& value);
    // /// Set a float, Vector or Matrix attribute stored in a variant.
    // bool SetVectorVariant(const String& name, const Variant& value);
    /// Set a Matrix3 attribute.
    bool SetMatrix3(const String& name, const Matrix3& value);
    /// Set a Matrix3x4 attribute.
    bool SetMatrix3x4(const String& name, const Matrix3x4& value);
    /// Set a Matrix4 attribute.
    bool SetMatrix4(const String& name, const Matrix4& value);

    /// Return whether does not refer to an element or an XPath node.
    bool IsNull() const;
    /// Return whether refers to an element or an XPath node.
    bool NotNull() const;
    /// Return true if refers to an element or an XPath node.
    explicit operator bool() const;
    /// Return element name (or attribute name if it is an attribute only XPath query result).
    String GetName() const;
    /// Return whether has a child element.
    bool HasChild(const String& name) const;
    /// Return whether has a child element.
    bool HasChild(const char* name) const;
    /// Return child element, or null if missing.
    XMLElement GetChild(const String& name = String::EMPTY) const;
    /// Return child element, or null if missing.
    XMLElement GetChild(const char* name) const;
    /// Return next sibling element.
    XMLElement GetNext(const String& name = String::EMPTY) const;
    /// Return next sibling element.
    XMLElement GetNext(const char* name) const;
    /// Return parent element.
    XMLElement GetParent() const;
    /// Return number of attributes.
    unsigned GetNumAttributes() const;
    /// Return whether has an attribute.
    bool HasAttribute(const String& name) const;
    /// Return whether has an attribute.
    bool HasAttribute(const char* name) const;
    /// Return inner value, or empty if missing for nodes like <node>value</node>.
    String GetValue() const;
    /// Return attribute, or empty if missing.
    String GetAttribute(const String& name = String::EMPTY) const;
    /// Return attribute, or empty if missing.
    String GetAttribute(const char* name) const;
    /// Return attribute as C string, or null if missing.
    const char* GetAttributeCString(const char* name) const;
    /// Return attribute in lowercase, or empty if missing.
    String GetAttributeLower(const String& name) const;
    /// Return attribute in lowercase, or empty if missing.
    String GetAttributeLower(const char* name) const;
    /// Return attribute in lowercase, or empty if missing.
    String GetAttributeUpper(const String& name) const;
    /// Return attribute in lowercase, or empty if missing.
    String GetAttributeUpper(const char* name) const;
    /// Return names of all attributes.
    std::vector<String> GetAttributeNames() const;
    /// Return bool attribute, or false if missing.
    bool GetBool(const String& name) const;
    // /// Return buffer attribute, or empty if missing.
    // std::vector<unsigned char> GetBuffer(const String& name) const;
    /// Copy buffer attribute into a supplied buffer. Return true if buffer was large enough.
    bool GetBuffer(const String& name, void* dest, unsigned size) const;
    /// Return bounding box attribute, or empty if missing.
    BoundingBox GetBoundingBox() const;
    /// Return a color attribute, or default if missing.
    Color GetColor(const String& name) const;
    /// Return a float attribute, or zero if missing.
    float GetFloat(const String& name) const;
    /// Return a double attribute, or zero if missing.
    double GetDouble(const String& name) const;
    /// Return an unsigned integer attribute, or zero if missing.
    unsigned GetUInt(const String& name) const;
    /// Return an integer attribute, or zero if missing.
    int GetInt(const String& name) const;
    /// Return an unsigned long long integer attribute, or zero if missing.
    unsigned long long GetUInt64(const String& name) const;
    /// Return a long long integer attribute, or zero if missing.
    long long GetInt64(const String& name) const;
    /// Return an IntRect attribute, or default if missing.
    IntRect GetIntRect(const String& name) const;
    /// Return an IntVector2 attribute, or default if missing.
    IntVector2 GetIntVector2(const String& name) const;
    /// Return an IntVector3 attribute, or default if missing.
    IntVector3 GetIntVector3(const String& name) const;
    /// Return a Rect attribute, or default if missing.
    Rect GetRect(const String& name) const;
    /// Return a quaternion attribute, or default if missing.
    Quaternion GetQuaternion(const String& name) const;
    // /// Return a variant attribute, or empty if missing.
    // Variant GetVariant() const;
    // /// Return a variant attribute with static type.
    // Variant GetVariantValue(VariantType type, Context* context = nullptr) const;
    // /// Return a resource reference attribute, or empty if missing.
    // ResourceRef GetResourceRef() const;
    // /// Return a resource reference list attribute, or empty if missing.
    // ResourceRefList GetResourceRefList() const;
    // /// Return a variant vector attribute, or empty if missing.
    // VariantVector GetVariantVector() const;
    /// Return a string vector attribute, or empty if missing.
    std::vector<String> GetStringVector() const;
    // /// Return a variant map attribute, or empty if missing.
    // VariantMap GetVariantMap() const;
    /// Return a Vector2 attribute, or zero vector if missing.
    Vector2 GetVector2(const String& name) const;
    /// Return a Vector3 attribute, or zero vector if missing.
    Vector3 GetVector3(const String& name) const;
    /// Return a Vector4 attribute, or zero vector if missing.
    Vector4 GetVector4(const String& name) const;
    /// Return any Vector attribute as Vector4. Missing coordinates will be zero.
    Vector4 GetVector(const String& name) const;
    // /// Return a float, Vector or Matrix attribute as Variant.
    // Variant GetVectorVariant(const String& name) const;
    /// Return a Matrix3 attribute, or zero matrix if missing.
    Matrix3 GetMatrix3(const String& name) const;
    /// Return a Matrix3x4 attribute, or zero matrix if missing.
    Matrix3x4 GetMatrix3x4(const String& name) const;
    /// Return a Matrix4 attribute, or zero matrix if missing.
    Matrix4 GetMatrix4(const String& name) const;
    ///
    template<typename T>
    T Get() {
        SE_LOG_WARNING("Type {}: don't implemented. Create:"
            "template<> YOUR_TYPE XMLElement::Set(const String& name) { \n\t//implementation\n}})"
            , typeid(T).name());
    }

    ///
    template<typename T>
    bool Set(const T& value) {
        SE_LOG_WARNING("Type {}: don't implemented. Create:"
            "template<> bool XMLElement::Set(const YOUR_TYPE& value) { \n\t//implementation\n}})"
            , typeid(T).name());
    }
    /// Return XML file.
    XMLFile* GetFile() const;

    /// Return pugixml xml_node_struct.
    pugi::xml_node_struct* GetNode() const { return node_; }

    /// Return XPath query result set.
    const XPathResultSet* GetXPathResultSet() const { return xpathResultSet_; }

    /// Return pugixml xpath_node.
    const pugi::xpath_node* GetXPathNode() const { return xpathNode_; }

    /// Return current result index.
    unsigned GetXPathResultIndex() const { return xpathResultIndex_; }

    /// Return next XPath query result. Only valid when this instance of XMLElement is itself one of the query result in the result set.
    XMLElement NextResult() const;

    /// Empty XMLElement.
    static const XMLElement EMPTY;

private:

    bool IsNullWithFile() const
    {
        return !file_ || (!node_ && !xpathNode_);
    }

    /// XML file.
    XMLFile* file_;
    /// Pugixml node.
    pugi::xml_node_struct* node_;
    /// XPath query result set.
    const XPathResultSet* xpathResultSet_;
    /// Pugixml xpath_node.
    const pugi::xpath_node* xpathNode_;
    /// Current XPath query result index (used internally to advance to subsequent query result).
    mutable unsigned xpathResultIndex_;
};

/// XPath query result set.
class XPathResultSet //GFROST_API
{
public:
    /// Construct empty result set.
    XPathResultSet();
    /// Construct with result set from XPath query.
    XPathResultSet(XMLFile* file, pugi::xpath_node_set* resultSet);
    /// Copy-construct.
    XPathResultSet(const XPathResultSet& rhs);
    /// Destruct.
    ~XPathResultSet();
    /// Assignment operator.
    XPathResultSet& operator =(const XPathResultSet& rhs);
    /// Return the n-th result in the set. Call XMLElement::GetNextResult() to get the subsequent result in the set.
    /// Note: The XPathResultSet return value must be stored in a lhs variable to ensure the underlying xpath_node_set* is still valid while performing XPathResultSet::FirstResult(), XPathResultSet::operator [], and XMLElement::NextResult().
    XMLElement operator [](unsigned index) const;
    /// Return the first result in the set. Call XMLElement::GetNextResult() to get the subsequent result in the set.
    /// Note: The XPathResultSet return value must be stored in a lhs variable to ensure the underlying xpath_node_set* is still valid while performing XPathResultSet::FirstResult(), XPathResultSet::operator [], and XMLElement::NextResult().
    XMLElement FirstResult();
    /// Return size of result set.
    unsigned Size() const;
    /// Return whether result set is empty.
    bool Empty() const;

    /// Return pugixml xpath_node_set.
    pugi::xpath_node_set* GetXPathNodeSet() const { return resultSet_; }

private:
    /// XML file.
    XMLFile* file_;
    /// Pugixml xpath_node_set.
    pugi::xpath_node_set* resultSet_;
};

/// XPath query.
class SE_API XPathQuery
{
public:
    /// Construct empty.
    XPathQuery();
    /// Construct XPath query object with query string and variable string. The variable string format is "name1:type1,name2:type2,..." where type is one of "Bool", "Float", "String", "ResultSet".
    explicit XPathQuery(const String& queryString, const String& variableString = String::EMPTY);
    /// Destruct.
    ~XPathQuery();
    /// Bind query object with variable set.
    void Bind();
    /// Add/Set a bool variable. Return true if successful.
    bool SetVariable(const String& name, bool value);
    /// Add/Set a float variable. Return true if successful.
    bool SetVariable(const String& name, float value);
    /// Add/Set a string variable. Return true if successful.
    bool SetVariable(const String& name, const String& value);
    /// Add/Set a string variable. Return true if successful.
    bool SetVariable(const char* name, const char* value);
    /// Add/Set an XPath query result set variable. Return true if successful.
    bool SetVariable(const String& name, const XPathResultSet& value);
    /// Set XPath query string and variable string. The variable string format is "name1:type1,name2:type2,..." where type is one of "Bool", "Float", "String", "ResultSet".
    bool SetQuery(const String& queryString, const String& variableString = String::EMPTY, bool bind = true);
    /// Clear by removing all variables and XPath query object.
    void Clear();
    /// Evaluate XPath query and expecting a boolean return value.
    bool EvaluateToBool(const XMLElement& element) const;
    /// Evaluate XPath query and expecting a float return value.
    float EvaluateToFloat(const XMLElement& element) const;
    /// Evaluate XPath query and expecting a string return value.
    String EvaluateToString(const XMLElement& element) const;
    /// Evaluate XPath query and expecting an XPath query result set as return value.
    /// Note: The XPathResultSet return value must be stored in a lhs variable to ensure the underlying xpath_node_set* is still valid while performing XPathResultSet::FirstResult(), XPathResultSet::operator [], and XMLElement::NextResult().
    XPathResultSet Evaluate(const XMLElement& element) const;

    /// Return query string.
    String GetQuery() const { return queryString_; }

    /// Return pugixml xpath_query.
    pugi::xpath_query* GetXPathQuery() const { return query_.get(); }

    /// Return pugixml xpath_variable_set.
    pugi::xpath_variable_set* GetXPathVariableSet() const { return variables_.get(); }

private:
    /// XPath query string.
    String queryString_;
    /// Pugixml xpath_query.
    std::unique_ptr<pugi::xpath_query> query_;
    /// Pugixml xpath_variable_set.
    std::unique_ptr<pugi::xpath_variable_set> variables_;
};

} // namespace Se

#if __has_include(<SeEngine/Core/Variant.h>)
#include <SeEngine/Core/Variant.h>
#include <SeEngine/Core/Serializable.h>
#include <SeEngine/Core/Context.h>
namespace Se
{

template<> bool XMLElement::Set(const Variant& value);
template<> Variant XMLElement::Get();


template<> inline bool XMLElement::Set(const ResourceRef& value) 
{
    if (IsNullWithFile())
        return false;

    // Need the context to query for the type
    String typeName = value.type_;
    // Context* context = file_->GetContext();
    // String typeName = String(context->GetTypeName(value.type_));
    return SetAttribute("value", String(typeName + ";" + value.name_).c_str());

}

template<> inline ResourceRef XMLElement::Get()
{
    ResourceRef ret;

    auto values = GetAttribute("value").split(';');
    if (values.size() == 2)
    {
        ret.type_ = values[0];
        ret.name_ = values[1];
    }

    return ret;
}


template<> inline bool XMLElement::Set(const ResourceRefList& value) 
{
    if (IsNullWithFile())
        return false;

    // Need the context to query for the type
    String str(value.type_);
    // auto context = Context::GetInstance();
    // String str(context->GetTypeName(value.type_));
    for (unsigned i = 0; i < value.names_.size(); ++i)
    {
        str += ";";
        str += value.names_[i];
    }

    return SetAttribute("value", str.c_str());

}

template<> inline ResourceRefList XMLElement::Get()
{
    ResourceRefList ret;

    auto values = GetAttribute("value").split(';', true);
    if (values.size() >= 1)
    {
        ret.type_ = values[0];
        ret.names_.resize(values.size() - 1);
        for (auto i = 1; i < values.size(); ++i)
            ret.names_[i - 1] = values[i];
    }

    return ret;
}


template<> inline bool XMLElement::Set(const VariantVector& value) 
{
    // Must remove all existing variant child elements (if they exist) to not cause confusion
    if (!RemoveChildren("variant"))
        return false;

    for (auto i = value.begin(); i != value.end(); ++i)
    {
        XMLElement variantElem = CreateChild("variant");
        if (!variantElem)
            return false;
        variantElem.Set<Variant>(*i);
    }

    return true;
}

template<> inline VariantVector XMLElement::Get()
{
    VariantVector ret;

    XMLElement variantElem = GetChild("variant");
    while (variantElem)
    {
        ret.push_back(variantElem.Get<Variant>());
        variantElem = variantElem.GetNext("variant");
    }

    return ret;
}


template<> inline bool XMLElement::Set(const VariantMap& value) 
{
    if (!RemoveChildren("variant"))
        return false;

    for (auto i = value.begin(); i != value.end(); ++i)
    {
        XMLElement variantElem = CreateChild("variant");
        if (!variantElem)
            return false;
        variantElem.SetUInt("hash", i->first.Value());
        variantElem.Set<Variant>(i->second);
    }

    return true;
}

template<> inline VariantMap XMLElement::Get()
{
    VariantMap ret;

    XMLElement variantElem = GetChild("variant");
    while (variantElem)
    {
        // If this is a manually edited map, user can not be expected to calculate hashes manually. Also accept "name" attribute
        if (variantElem.HasAttribute("name"))
            ret[StringHash(variantElem.GetAttribute("name"))] = variantElem.Get<Variant>();
        else if (variantElem.HasAttribute("hash"))
            ret[StringHash(variantElem.GetUInt("hash"))] = variantElem.Get<Variant>();

        variantElem = variantElem.GetNext("variant");
    }

    return ret;
}

/*
template<> inline bool XMLElement::Set(const Variant& value) 
{

    return false;
}

template<> inline Variant XMLElement::Get(const String& value)
{
    //Variant ret;

    return {};
}
*/

template<> inline Variant XMLElement::Get()
{
    VariantType type = Variant::GetTypeFromName(GetAttribute("type"));
    //return GetVariantValue(type);
    Variant ret;

    if (type == VAR_RESOURCEREF)
        ret = Get<ResourceRef>();
    else if (type == VAR_RESOURCEREFLIST)
        ret = Get<ResourceRefList>();
    else if (type == VAR_VARIANTVECTOR)
        ret = Get<VariantVector>();
    else if (type == VAR_STRINGVECTOR)
        ret = GetStringVector();
    else if (type == VAR_VARIANTMAP)
        ret = Get<VariantMap>();
    else if (type == VAR_CUSTOM)
    {
        auto context = Context::GetInstance();
        if (!context)
        {
            SE_LOG_ERROR("Context must not be null for SharedPtr<Serializable>");
            return ret;
        }

        const String& typeName = GetAttribute("type");
        if (!typeName.empty())
        {
            SharedPtr<Serializable> object;
            object.StaticCast(context->CreateObject(typeName));

            if (object != nullptr)
            {
                // Restore proper refcount.
                if (object->LoadXML(*this))
                    ret.SetCustom(object);
                else
                    SE_LOG_ERROR("Deserialization of '{}' failed", typeName);
            }
            else
                SE_LOG_ERROR("Creation of type '{}' failed because it has no factory registered", typeName);
        }
        else if (!GetChild().IsNull())
            SE_LOG_ERROR("Malformed xml input: 'type' attribute is required when deserializing an object");
    }
    else
        ret.FromString(type, GetAttributeCString("value"));

    return ret;
}

template<> inline bool XMLElement::Set(const Variant& value) 
{
    if (!SetAttribute("type", value.GetTypeName().c_str()))
        return false;

    switch (value.GetType())
    {
    case VAR_RESOURCEREF:
        return Set<ResourceRef>(value.Get<ResourceRef>());

    case VAR_RESOURCEREFLIST:
        return Set<ResourceRefList>(value.Get<ResourceRefList>());

    case VAR_VARIANTVECTOR:
        return Set<VariantVector>(value.Get<VariantVector>());

    case VAR_STRINGVECTOR:
        return SetStringVector(value.GetStringVector());

    case VAR_VARIANTMAP:
        return Set<VariantMap>(value.Get<VariantMap>());

    default:
        return SetAttribute("value", value.ToString().c_str());
    }
    return false;
}

} // namespace Se
#endif


