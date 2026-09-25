#pragma once

#include <exception>
#ifndef SE_REFLACTION_STANDALONE
#define SE_REFLACTION_STANDALONE
#endif

#if defined(SE_REFL_ATTRIBUTES_BLOCK) && !defined(IGNORE_ATTRIBUTES_BLOCK)
#define SE_REFL_ATTRIBUTES_BLOCK "Attributes"
#endif

//#if __has_include("Se/String.hpp") && !SE_REFLACTION_STANDALONE
#include <Se/String.hpp>
#include <Se/Console.hpp>
// #else
// #include <string>
// using String = std::string;
// #endif
#include <memory>
#include <functional>

#if !defined(SE_REFLACTION_STANDALONE)
#  include <SeArc/ArchiveSerialization.hpp>

namespace Se {

#else

namespace Se {

class ReflectedObject;

class ReflArchive {
public:
    ReflArchive(bool humanReadable, bool isInput)
        : humanReadable_(humanReadable)
        , isInput_(isInput){}

    virtual ~ReflArchive() = default;

    virtual void Serialize(const std::string& name, ReflectedObject& value) = 0;

    bool IsHumanReadable() const { return humanReadable_; }

    bool IsInput() const { return isInput_; };

private:
    bool humanReadable_;
    bool isInput_;
    //virtual void SerializeInBlock(Archive& archive, const std::string& name) {}

};



template<typename T, typename = void>
struct HasSerializeInBlock : std::false_type {};

template<typename T>
struct HasSerializeInBlock<T, 
    std::void_t<decltype(std::declval<T>().SerializeInBlock(
        std::declval<ReflArchive&>(), 
        std::declval<const std::string&>()
    ))>> : std::true_type {};


template<typename T>
void SerializeValue(ReflArchive& archive, const char* name, T& value)
{
    if constexpr (HasSerializeInBlock<T>::value) {
        value.SerializeInBlock(archive, name);
        return;
    }

    assert(0 && "SeArc Serialization is disabled");
}

inline void ReflArchive::Serialize(const std::string& name, ReflectedObject& value) {
    //ToStringTypeId<T>()
}

// void Archive::Serialize(const std::string& name, ReflectedObject& value)
// {

// }

#endif

class Archive;

enum class AttributeType {
    AT_None, 
    AT_Parameter, // Maybe remove, because AttributeValue has functional like AttributeAccessor
    AT_Enum,
    AT_Accessor,
//    AT_EnumAccessor,
    AT_Action,
    AT_Link, // TODO same type objects use one resource
//    AT_Switcher

};

enum class AttributeParameterType {
    AT_None, 
    AT_INT,
    AT_FLOAT,
    AT_BOOL,
    AT_DOUBLE,
    AT_OBJECT,
    AT_ENUM
//    AT_EnumAccessor,
//    AT_Action,
//    AT_Switcher

};

class AttributeInfo 
{
public:
    AttributeInfo(AttributeType type) 
        : type_(type) {}

    virtual ~AttributeInfo() = default;

    virtual String GetTypeName() const {
        return ""; }

    virtual bool IsAction() const { return type_ == AttributeType::AT_Action; }

    /// Call function if attribute AttributeType::AT_Action
    virtual bool Call() const { return false; }
    /// AttributeType: AT_None, AT_Parameter, AT_Accessor, AT_EnumAccessor, AT_Action
    AttributeType GetType() const {
        return type_; }

    virtual std::size_t GetValueType() { 
            return 0; }
    virtual void SerializeInBlock(Se::Archive& archive, const String& name) {}

    virtual const std::vector<String> GetEnumNames() const { 
        return {}; }


    virtual void ToDefault() {}

protected:
    AttributeType type_{AttributeType::AT_None};

};

/// Abstract base class for invoking attribute accessors.
template<class ParameterType>
class AttributeAccessor : public AttributeInfo {
public:
    /// Construct.
    AttributeAccessor(ParameterType defaultValue, AttributeType type = AttributeType::AT_Accessor) 
        : AttributeInfo(type)
        , defaultValue_(defaultValue)
    {

    }

    std::size_t GetValueType() override { 
        return typeid(ParameterType).hash_code(); }

    /// Get the attribute.
    virtual void Get(ParameterType* dest) const = 0;
    /// Set the attribute.
    virtual void Set(const ParameterType& src) = 0;
    
    String GetTypeName() const override {
        return typeName_;
    }

    virtual ParameterType* GetPtr() {
        return nullptr;
    }

    virtual bool IsDefault() const {
        return false;
    }

    ParameterType GetDefaultValue() const {
        return defaultValue_;
    }

    void ToDefault() override {
        this->Set(defaultValue_);
    }

    template<class T> 
    AttributeAccessor<T>* AccesorCast() {
        return reinterpret_cast<AttributeAccessor<T>*>(this);
    }


protected:
    /// @brief TODO
    ParameterType defaultValue_;

    std::string typeName_{ToStringTypeId<ParameterType>()};
};

class AttributeEmpty : public AttributeAccessor<char>
{
public:
    using EmptyType = char;
    //AttributeEmptyValue emptyValue;
    /// Construct.
    AttributeEmpty() : AttributeAccessor<EmptyType>(0) {}
    ~AttributeEmpty() override = default;
    /// Get the attribute.
    void Get(EmptyType* dest) const override {};
    /// Set the attribute.
    void Set(const EmptyType& src) override {};
};

typedef std::shared_ptr<Se::AttributeEmpty> AttributePtr;

template<class ParameterType>
class AttributeValue : public AttributeAccessor<ParameterType> {
public:
    /// Construct.
    AttributeValue(ParameterType* value, ParameterType defaultValue) 
        : AttributeAccessor<ParameterType>(defaultValue)
        , value_(value)
    {        
        
    }
    /// Get the attribute.
    void Get(ParameterType* dest) const override {
        *dest = *value_;
    };
    /// Set the attribute.
    void Set(const ParameterType& src) override {
        *value_ = src;
    };

    bool IsDefault() const override {
        return this->defaultValue_ == *value_;
    }

    ParameterType* GetPtr() override {
        return value_;
    }
    void SerializeInBlock(Se::Archive& archive, const String& name) override;
private:
    ParameterType* value_;
};

template<class ParameterType>
class AttributeEnum : public AttributeAccessor<ParameterType> 
{
public:
    /// Construct for with dynamic change list
    AttributeEnum(ParameterType* value, ParameterType defaultValue, std::vector<String>* names) 
        : AttributeAccessor<ParameterType>(defaultValue, AttributeType::AT_Enum)
        , value_(value)
        , namesPtr_(names)
        , dynamic_(true)
    {        
    }

    /// Construct for static change list
    AttributeEnum(ParameterType* value, ParameterType defaultValue, const std::vector<String>& names) 
        : AttributeAccessor<ParameterType>(defaultValue, AttributeType::AT_Enum)
        , value_(value)
        , names_(names)
    {        
    }

    /// Get the attribute.
    void Get(ParameterType* dest) const override {
        *dest = *value_;
    };
    /// Set the attribute.
    void Set(const ParameterType& src) override {
        *value_ = src;
    };

    bool IsDefault() const override {
        return this->defaultValue_ == *value_;
    }

    const std::vector<String> GetEnumNames() const override {
        return namesPtr_ ? *namesPtr_ : names_; }
    void SerializeInBlock(Se::Archive& archive, const String& name) override;
private:
    ParameterType* value_;
    std::vector<String> names_;
    std::vector<String>* namesPtr_;
    bool dynamic_{false};
};

/// Template implementation of the variant attribute accessor.
template <class ParameterType, class TGetFunction, class TSetFunction>
class AttributeAccessorImpl : public AttributeAccessor<ParameterType>
{
public:
    /// Construct.
    AttributeAccessorImpl(TGetFunction getFunction, TSetFunction setFunction, ParameterType defaultValue) 
        : AttributeAccessor<ParameterType>(defaultValue)
        , getFunction_(getFunction)
        , setFunction_(setFunction)
    {
    }

    /// Invoke getter function.
    void Get(ParameterType* value) const override
    {
        assert(value);
        *value = getFunction_();
    }

    /// Invoke setter function.
    void Set(const ParameterType& value) override
    {
        setFunction_(value);
    }
    void SerializeInBlock(Se::Archive& archive, const String& name) override;


    bool IsDefault() const override {
        return this->defaultValue_ == getFunction_();
    }

private:
    /// Get functor.
    TGetFunction getFunction_;
    /// Set functor.
    TSetFunction setFunction_;
};

#ifdef USE_ARCHIVE_SERIALIZATION
template<class ParameterType>
inline void AttributeValue<ParameterType>::SerializeInBlock(Se::Archive& archive, const String& name)
{
    SerializeValue(archive, name, *value_);
}

template<class ParameterType>
inline void AttributeEnum<ParameterType>::SerializeInBlock(Se::Archive& archive, const String& name)
{
    SerializeValue(archive, name, *value_);
}

template<class ParameterType, class TGetFunction, class TSetFunction>
inline void AttributeAccessorImpl<ParameterType, TGetFunction, TSetFunction>::SerializeInBlock(Se::Archive& archive, const String& name)
{
    ParameterType value = getFunction_();
    SerializeValue(archive, name, value);

    if (archive.IsInput())
        setFunction_(value);
}
#else
template<class ParameterType>
inline void AttributeValue<ParameterType>::SerializeInBlock(Se::Archive&, const String&) {}

template<class ParameterType>
inline void AttributeEnum<ParameterType>::SerializeInBlock(Se::Archive&, const String&) {}

template<class ParameterType, class TGetFunction, class TSetFunction>
inline void AttributeAccessorImpl<ParameterType, TGetFunction, TSetFunction>::SerializeInBlock(Se::Archive&, const String&) {}
#endif

// template <class ParameterType>
// class AttributeEnumImpl : public AttributeAccessor<ParameterType>
// {
// public:

//     /// Construct.
//     AttributeEnumImpl(ParameterType* value, ParameterType defaultValue, const std::vector<String>& enumNames) 
//         : AttributeAccessor<ParameterType>(defaultValue, AttributeType::AT_Enum)
//         , enumNames_(enumNames)
//     {        
        
//     }
//     /// Get the attribute.
//     void Get(ParameterType* dest) const override {
//         *dest = *value_;
//     };
//     /// Set the attribute.
//     void Set(const ParameterType& src) override {
//         *value_ = src;
//     };



// private:
//     const std::vector<String>& enumNames_;

// };

template <class ParameterType, class TGetFunction, class TSetFunction>
class AttributeAccessorEnumImpl : public AttributeAccessorImpl<ParameterType, TGetFunction, TSetFunction>
{
public:
    AttributeAccessorEnumImpl(TGetFunction getFunction, TSetFunction setFunction, const std::vector<String>& enumNames, ParameterType defaultValue) 
            : AttributeAccessorImpl<TGetFunction, TSetFunction, TSetFunction>(getFunction, setFunction, defaultValue) 
            , enumNames_(enumNames)
    {

    }

    const std::vector<String> GetEnumNames() override {
        return enumNames_; }

private:
    const std::vector<String>& enumNames_;

};

template <class TFunction>
class AttributeActionImpl : public AttributeInfo
{
public:
    /// Construct.
    AttributeActionImpl(TFunction func)
        : AttributeInfo(AttributeType::AT_Action)
        , function_(func)
    {
    }

    bool Call() const override
    {
        function_();
        return true;
    }

private:
    /// functor.
    TFunction function_;
};

template<class T>
class Reflected;

class Attributes
{

public:
    Attributes() {}

    template<class T>
    void Register(const String& name, T* value, const T& defaultValue = T())
    {
        auto ptr = new AttributeValue<T>(value, defaultValue);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
        // auto ptr = std::make_shared<AttributeValue<T>>(value, defaultValue);
        // values_[name] = std::dynamic_pointer_cast<AttributeEmpty>(ptr);
        //ptr->ToDefault();
    }

    template<class T>
    void RegisterEnum(const String& name, T* value,
            const std::vector<String>& enumsString, T defaultValue = T{})
    {
        auto ptr = new AttributeEnum(value, defaultValue, enumsString);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
    }

    template<class T>
    void RegisterEnum(const String& name, T* value,
            std::vector<String>* enumsString, T defaultValue = T{})
    {
        auto ptr = new AttributeEnum(value, defaultValue, enumsString);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
    }


    template<class T, class ObjectType, class TGetFunction, class TSetFunction>
    void RegisterEnum(const String& name, ObjectType* object, TGetFunction getFunction, TSetFunction setFunction, 
            const std::vector<String>& enumsString, T defaultValue = T{})
    {
        auto funcGet = std::bind(getFunction, object);
        auto funcSet = std::bind(setFunction, object, std::placeholders::_1);
        auto ptr = new AttributeAccessorImpl<T, decltype(funcGet), decltype(funcSet)>(funcGet, funcSet, defaultValue);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
        //ptr->ToDefault();
    }

    template<class T, class ObjectType, class TGetFunction, class TSetFunction>
    void Register(const String& name, ObjectType* object, TGetFunction getFunction, TSetFunction setFunction, T defaultValue = T())
    {
        auto funcGet = std::bind(getFunction, object);
        auto funcSet = std::bind(setFunction, object, std::placeholders::_1);
        auto ptr = new AttributeAccessorImpl<T, decltype(funcGet), decltype(funcSet)>(funcGet, funcSet, defaultValue);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
        //ptr->ToDefault();
    }

    template<class ObjectType, class TFunction>
    void RegisterAction(const String& name, ObjectType* object, TFunction func)
    {
        auto bindFunc = std::bind(func, object);
        auto ptr = new AttributeActionImpl<decltype(bindFunc)>(bindFunc);
        values_.emplace_back(name, std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr)));
    }

    std::vector<String> GetAttriburesNames() const {
        std::vector<String> ret;
        for (auto v : values_)
            ret.push_back(v.first);
        return ret;
    }

    void ToDefault()
    {
        // if (values_.empty() || values_.size() < 1)
        //     return;

        for (auto& val : values_)
            if (val.second->GetType() == AttributeType::AT_Accessor)
                val.second->ToDefault();
    }

    std::shared_ptr<AttributeEmpty> FindAttribute(const String& name) {
        for (auto attr : values_)
            if (attr.first == name)
                return attr.second;

        //SE_LOG_ERROR("Object has no attribute : {}", name);
        return nullptr;
    }

    template<class T>
    T Get(const String& name)
    {
        auto attr = FindAttribute(name);

        if (!attr)
            return {};

        T value{0};
        reinterpret_cast<AttributeAccessor<T>*>(attr.get())->Get(&value);
        return value;
    }

    template<class T>
    void Set(const String& name, const T& value)
    {
        auto attr = FindAttribute(name);

        if (!attr)
            return;

        auto rcast = reinterpret_cast<AttributeValue<T>*>(attr.get());

        if (attr->GetTypeName() == "float" && ToStringTypeId<T>() == "double") {//TODO simplify
            rcast->Set(static_cast<float>(value));
        }
        else
            rcast->Set(value);
    }

    bool Call(const String& name)
    {
        auto attr = FindAttribute(name);

        if (!attr || !(attr->GetType() == AttributeType::AT_Action))
            return false;

        return attr->Call();
    }
    virtual void SerializeInBlock(Archive& archive);
protected:
    std::vector<std::pair<String, std::shared_ptr<AttributeEmpty>>> values_;
    inline static std::unordered_map<String, std::shared_ptr<AttributeEmpty>> staticValues_{};
};

#ifdef USE_ARCHIVE_SERIALIZATION
inline void Attributes::SerializeInBlock(Archive& archive)
{
    for (auto& attr : values_)
        attr.second->SerializeInBlock(archive, attr.first);
}
#else
inline void Attributes::SerializeInBlock(Archive&) {}
#endif

template <typename T, typename = std::void_t<>>
struct HasRegisterAttributes : std::false_type {};

template <typename T>
struct HasRegisterAttributes<T, 
    std::void_t<decltype(std::declval<T>().template RegisterAttributes(std::declval<Se::Attributes&>()))>> 
    : std::true_type {};

template<class T>
void IsReflected(T* refl, Se::Attributes* attr)
{
    if constexpr (HasRegisterAttributes<T>::value) {
        refl->RegisterAttributes(*attr);
        return;
    }

    SE_LOG_WARNING("Do not registered Attributes for type: {}", ToStringTypeId<T>());
}

template<class T>
bool ReflectionRegisterAttributes(T* refl, Se::Attributes* attr)
{
    if constexpr (HasRegisterAttributes<T>::value) {
        if (attr)
            refl->RegisterAttributes(*attr);
        return true;
    }

    return false;
}

class ReflectedObject;

template<class T>
class Reflected //: public std::enable_shared_from_this<Reflected<T>>
{

public:
    using ReflectedType = T;

    Reflected()
        : type_(ToStringTypeId<T>())
    {
        //Reflected* ptr = this;
        object_ = new T();// reinterpret_cast<T*>(ptr);
        baseType_ = ToStringTypeId<decltype(*object_)>(); // GetTypeOrig();

        if (!ReflectionRegisterAttributes(object_, &attributes_))
            SE_LOG_WARNING("Do not registered Attributes for type: {}", ToStringTypeId<T>());

        generated_ = true;

        attributes_.ToDefault();
    }

    Reflected(T* obj, bool generated = false)
        : type_(ToStringTypeId<T>())
    {

        
        object_ = obj;
        baseType_ = ToStringTypeId<decltype(*obj)>(); // GetTypeOrig();

        if (!ReflectionRegisterAttributes(object_, &attributes_))
            SE_LOG_WARNING("Do not registered Attributes for type: {}", ToStringTypeId<T>());
        generated_ = generated;

        if (generated)
            attributes_.ToDefault();

    }

    virtual ~Reflected()
    {
        if (generated_)
            delete object_;
    }
    template<typename... Args>
    static std::shared_ptr<ReflectedObject> ToReflectedObject(T* object) {

        // auto obj = new Se::Reflected<T>(object, false);
        // ReflectionRegisterAttributes(obj, &obj->attributes_);
        // return std::shared_ptr<Se::ReflectedObject>(reinterpret_cast<Se::ReflectedObject*>(obj));

        auto obj = new Se::Reflected<T>(object, false);
        return std::shared_ptr<Se::ReflectedObject>(reinterpret_cast<Se::ReflectedObject*>(obj));
    }


    template<typename... Args>
    static std::shared_ptr<ReflectedObject> CreateReflectedObject(Args&&... args) {

        auto obj = std::make_shared<Se::Reflected<T>>(std::forward<Args>(args)...);
        obj->generated_ = true;
        return std::dynamic_pointer_cast<Se::ReflectedObject>(std::move(obj));

        // auto obj = new Se::Reflected<T>(new T(std::forward<Args>(args)...), true);
        // ReflectionRegisterAttributes(obj, &obj->attributes_);
        // return std::shared_ptr<Se::ReflectedObject>(reinterpret_cast<Se::ReflectedObject*>(obj));
    }

    virtual void SerializeInBlock(Archive& archive);

    template<class U> 
    U* GetObject() {
        return reinterpret_cast<U*>(object_);
    }

    template<class U>
    void RegisterAttributesParrent(const String& base)
    {
        baseType_ = base;
        //this->U::RegisterAttributes(attributes_);
        //baseType_ = ToStringTypeId<U>();
        //dynamic_cast<U*>(this)->RegisterAttributes(attributes);
        //this->RegisterAttributes(attributes);
    }

    std::vector<String> GetAttriburesNames() {
        return attributes_.GetAttriburesNames();
    }

    // virtual void RegisterAttributes(Attributes& attributes)
    // {
    //     //SE_LOG_WARNING("{} attributes is empty", type_);
    //     //ReflectionRegisterAttributes(object_, attributes_);
    // }

    std::shared_ptr<AttributeEmpty> FindAttribute(const String& name) {
        return attributes_.FindAttribute(name);
    }

    template<class U>
    U GetAttribute(const String& name)
    {
        return attributes_.template Get<U>(name);
    }

    template<class U>
    void SetAttribute(const String& name, const U& value)
    {
        return attributes_.Set(name, value);
    } 

    static std::shared_ptr<T> Create() {
        auto newObject = std::make_shared<T>();
        //newObject->baseType_ = ToStringTypeId<T>();
        auto attributes = Reflected<T>::CreateReflectedObject()->attributes_;
        if (!ReflectionRegisterAttributes<T>(newObject.get(), &attributes))
            SE_LOG_WARNING("Do not registered Attributes for type: {}", ToStringTypeId<T>());
        //newObject->initAttributes<T>();
        return newObject;
    }

    virtual String GetTypeOrig() {
        static String staticTypeOrig{};
        staticTypeOrig = ToStringTypeId<ReflectedType>();
        //return typeid(decltype(*this)).name();
        return baseType_;
    }

    virtual String GetType() {
        return type_;
    }

    template<class Parent>
    bool IsClassBase() {
        String outString;
        outString = format("-+ {}\n-+ {} {}", ToStringTypeId<Parent>(), GetType(), baseType_);
        outString += format("-+ {}\n-+ {}", baseType_, GetType()); 
        outString += format("-+ {}\n-+ {}", ToStringTypeId<T>(), GetType());
        SE_LOG_INFO(outString);
        return std::is_base_of<Parent, T>::value;
        //typeid(T) == typeid(U);
    }

    static String GetStaticType() {
        static String staticType{};
        if (!staticType.empty())
            return staticType;
        return ToStringTypeId<T>();
    }

    void initAttributes()
    {
        //RegisterAttributes(attributes_);
    }

protected:
    
    
    String type_;
    String baseType_;

    T* object_;
    bool generated_{false};
    
private:
    
    Attributes attributes_;
};

#ifdef USE_ARCHIVE_SERIALIZATION
template<class T>
inline void Reflected<T>::SerializeInBlock(Archive& archive)
{
#ifdef SE_REFL_ATTRIBUTES_BLOCK
    SerializeValue(archive, SE_REFL_ATTRIBUTES_BLOCK, attributes_);
#else
    attributes_.SerializeInBlock(archive);
#endif
}
#else
template<class T>
inline void Reflected<T>::SerializeInBlock(Archive&) {}
#endif

class ReflectedObject : public Reflected<ReflectedObject> 
{

public:
    template<class T>
    T* Cast() {
        return reinterpret_cast<T*>(this->object_);
    }

};

class ReflectedManager 
{
public:
    template<class T>
    static void Register()
    {
        std::shared_ptr<Reflected<T>>(*createFunc)()  = []() -> std::shared_ptr<Reflected<T>> 
        {
            return std::make_shared<Reflected<T>>(new T(), true);
        };

         registered_[ToStringTypeId<T>()] = 
             reinterpret_cast<std::shared_ptr<ReflectedObject>(*)()>(createFunc);
    }

    static std::shared_ptr<ReflectedObject> Create(const String& type)
    {
        auto func = registered_.find(type);
        if (func == registered_.end()) {
            // SE_LOG_WARNING("Type \"{}\" is not registered", type);
            return std::make_shared<ReflectedObject>();
            //return std::move(ret);
        }

        return func->second();
    }

    // template<class T>
    // static void ToAttibutesDefault(std::shared_ptr<Reflected<T>> lhs)
    // {
    //     auto it = registered_.find(type);
    //     for (auto callbackCreate : it->second) {
    //         auto ref = callbackCreate();
    //         //lhs.att
    //     }

    //     if ( == registered_.end()) {
    //         // SE_LOG_WARNING("Type \"{}\" is not registered", type);
    //         return registered_[ReflectedObject::GetStaticType()]();
    //     }

    //     return registered_[type]();
    // }

    // template<class T>
    // static std::shared_ptr<T> Create()
    // {
    //     auto reg = registered_[ToStringTypeId<T>()];

    //     if (!reg) {
    //         auto typeName = ToStringTypeId<T>();
    //         SE_LOG_ERROR(
    //             "ASSERT. {} - Doesn`t registered.\n"
    //             "Use in global scope: REGISTER_OBJECT_REFLECTED({});", typeName, typeName);
    //         assert(0);
    //     }


    //     //return std::dynamic_pointer_cast<T>(registered_[T::GetStaticType()]());
    //     return reinterpret_cast<std::shared_ptr<T>(*)()>(registered_[ToStringTypeId<T>()])();
    // }

    static std::unordered_map<String, std::shared_ptr<ReflectedObject>(*)()> registered_;
};

inline std::unordered_map<String, std::shared_ptr<ReflectedObject>(*)()> ReflectedManager::registered_ = {
    {ReflectedObject::GetStaticType(), &ReflectedObject::Create}
};

template<typename T>
struct RegistratorObjectReflected {
  RegistratorObjectReflected() {
    Se::ReflectedManager::Register<T>();

  }
};

#define REGISTER_OBJECT_REFLECTED(typeName) \
    namespace ReflectedRegScope { \
        static RegistratorObjectReflected<typeName> reg_##typeName = RegistratorObjectReflected<typeName>(); \
    }

}

