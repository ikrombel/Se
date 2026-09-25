#pragma once

#include "Se/String.hpp"
#include "SeMath/ArchiveMath.hpp"
#include "SeMath/Vector4.hpp"
#include <SeResource/JSONValue.h>
#include <SeReflection/Reflected.hpp>

#include <typeinfo>
#include <unordered_map>

namespace Se {

template<class T>
inline bool ParameterJSON(const char* label, T& value, JSONValue& json, bool serialization = false)
{
    return false;
}

template<>
inline bool ParameterJSON(const char* label, int& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value;
    else
        value = json.Get(label).GetInt();

    return true;
}

template<>
inline bool ParameterJSON(const char* label, float& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value;
    else
        value = json.Get(label).GetFloat();

    return true;
}

template<>
inline bool ParameterJSON(const char* label, bool& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value;
    else
        value = json.Get(label).GetBool();

    return true;
}

template<>
inline bool ParameterJSON(const char* label, long long & value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = static_cast<double>(value);
    else
        value = static_cast<long long>(json.Get(label).GetDouble());

    return true;
}

template<>
inline bool ParameterJSON(const char* label, String& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value;
    else
        value = json.Get(label).GetString();

    return true;
}

class ReflObjectJSON
{
public:
    using WrapParameterJSONFunc = bool(*)(const char*, AttributePtr&, JSONValue&, bool);

    template<class T>
    void Link(T* object) {
        reflected_ =  Reflected<T>::ToReflectedObject(object);
    }

    JSONValue ToJSON();

    template<typename T>
    std::shared_ptr<T> FromJSON(JSONValue& value);

    template<typename... Args>
    void Register(bool reset = false);

    static bool DispatchParameterJSON(const char* label, AttributePtr& attr, JSONValue& json, bool serialization);

    template<typename T>
    static bool WrapParameterJSON(const char* label, AttributePtr& attr, JSONValue& json, bool serialization);

protected:
    std::shared_ptr<ReflectedObject> reflected_;

    static std::unordered_map<std::size_t, WrapParameterJSONFunc> wrapParameterJSONRegister_;
};

inline bool ReflObjectJSON::DispatchParameterJSON(const char* label, AttributePtr& attr, JSONValue& json, bool serialization)
{
    if (!attr)
        return false;
    auto it = wrapParameterJSONRegister_.find(attr->GetValueType());
    if (it != wrapParameterJSONRegister_.end())
        return it->second(label, attr, json, serialization);
    SE_LOG_WARNING("Unknown attribute type, skipping: {}", attr->GetTypeName());
    return false;
}

template<typename T>
inline bool ReflObjectJSON::WrapParameterJSON(const char* label, AttributePtr& attr, JSONValue& json, bool serialization)
{
    auto attrAccesor = attr->AccesorCast<T>();

    T value{};
    attrAccesor->Get(&value);

    if (ReflectionRegisterAttributes(&value, nullptr))
    {
        JSONValue& nestedJson = json[label];
        auto obj = Se::Reflected<T>::ToReflectedObject(&value);
        auto attributeNames = obj->GetAttriburesNames();

        for (auto& attrName : attributeNames) {
            auto attrObj = obj->FindAttribute(attrName);
            if (!attrObj)
                continue;
            ReflObjectJSON::DispatchParameterJSON(attrName.c_str(), attrObj, nestedJson, serialization);
        }
        if (!serialization)
            attrAccesor->Set(value);
        return true;
    }
    else if (ParameterJSON<T>(label, value, json, serialization))
    {
        attrAccesor->Set(value);
    }
    else
        SE_LOG_TODO("Do not implemented for type: {}\n", ToStringTypeId<T>());

    return false;
}

template<typename... Args>
inline void ReflObjectJSON::Register(bool reset)
{
    if (reset)
    {
        wrapParameterJSONRegister_.clear();
        Register<int, float, bool, long long, Se::String>();
    }

    (void)std::initializer_list<int>{
        (wrapParameterJSONRegister_[typeid(Args).hash_code()] = &ReflObjectJSON::WrapParameterJSON<Args>, 0)...
    };
}

template<typename T>
std::shared_ptr<T> ReflObjectJSON::FromJSON(JSONValue& value)
{
    auto obj = std::make_shared<T>();

    Se::Attributes attrs;
    if (ReflectionRegisterAttributes(obj.get(), &attrs))
    {
        auto attributeNames = attrs.GetAttriburesNames();
        for (auto& attrName : attributeNames)
        {
            auto attr = attrs.FindAttribute(attrName);
            if (!attr || !value.Contains(attrName))
                continue;

            ReflObjectJSON::DispatchParameterJSON(attrName.c_str(), attr, value, false);
        }
    }

    return obj;
}

JSONValue ReflObjectJSON::ToJSON()
{
    JSONValue json;
    if (!reflected_)
        return json;

    auto attributeNames = reflected_->GetAttriburesNames();
    for (auto& attrName : attributeNames)
    {
        auto attr = reflected_->FindAttribute(attrName);
        if (!attr)
            continue;
        ReflObjectJSON::DispatchParameterJSON(attrName.c_str(), attr, json, true);
    }
    return json;
}

std::unordered_map<std::size_t, ReflObjectJSON::WrapParameterJSONFunc> ReflObjectJSON::wrapParameterJSONRegister_;

template<>
inline bool ParameterJSON(const char* label, Se::Vector2& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value.ToString();
    else
        value = ToVector2(json.Get(label).GetString());

    return true;
}

template<>
inline bool ParameterJSON(const char* label, Se::IntVector2& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value.ToString();
    else
        value = ToIntVector2(json.Get(label).GetString());

    return true;
}

template<>
inline bool ParameterJSON(const char* label, Se::Vector3& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value.ToString();
    else
        value = ToVector3(json.Get(label).GetString());

    return true;
}

template<>
inline bool ParameterJSON(const char* label, Se::IntVector3& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value.ToString();
    else
        value = ToIntVector3(json.Get(label).GetString());

    return true;
}

template<>
inline bool ParameterJSON(const char* label, Se::IntVector4& value, JSONValue& json, bool serialization)
{
    if (serialization)
        json[label] = value.ToString();
    else
        value = ToIntVector4(json.Get(label).GetString());

    return true;
}

template<typename T>
inline bool ParameterJSON(const char* label, std::vector<T>& value, JSONValue& json, bool serialization)
{
    if (serialization)
        for (auto v : value) {
            JSONValue vnode;
            if (ParameterJSON("", v, json, serialization))
                json[label].Push(vnode);
        }
    else
        for (auto v : json.GetArray()) {
            T vnode;
            if (ParameterJSON("", vnode, v, serialization))
                json[label].Push(vnode);
        }
    return true;
}



} // namespace Se 