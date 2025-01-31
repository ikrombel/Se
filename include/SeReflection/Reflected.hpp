#include <Se/String.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#include <functional>

namespace Se {

enum class AttributeType {
    AT_None, 
    AT_Parameter,
    AT_Accessor,
//    AT_EnumAccessor,
//    AT_Action,
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
    virtual String GetTypeName() const {
        return ""; }

    virtual bool IsAction() { return false; }
};

/// Abstract base class for invoking attribute accessors.
template<class ParameterType>
class AttributeAccessor : public AttributeInfo {
public:
    /// Construct.
    AttributeAccessor(ParameterType defaultValue) 
        : AttributeInfo()
        , defaultValue_(defaultValue)
    {

    }

    /// Get the attribute.
    virtual void Get(ParameterType* dest) const = 0;
    /// Set the attribute.
    virtual void Set(const ParameterType& src) = 0;

    virtual void SerializeInBlock(Se::Archive& archive, const String& name) = 0;
    
    String GetTypeName() const override {
        return type_;
    }

    virtual bool IsDefault() const {
        return false;
    }

    ParameterType GetDefaultValue() const {
        return defaultValue_;
    }

    void ToDefault() {
        this->Set(defaultValue_);
    }


protected:
    /// @brief TODO
    ParameterType defaultValue_;

    std::string type_{ToStringTypeId<ParameterType>()};
};

inline struct AttributeEmptyValue {}emptyValue;
class AttributeEmpty : public AttributeAccessor<AttributeEmptyValue>
{
    /// Construct.
    AttributeEmpty() : AttributeAccessor<AttributeEmptyValue>(emptyValue) {}
    /// Get the attribute.
    void Get(AttributeEmptyValue* dest) const override {};
    /// Set the attribute.
    void Set(const AttributeEmptyValue& src) override {};
};

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

    void SerializeInBlock(Se::Archive& archive, const String& name) override {
        SerializeValue(archive, name, *value_);
    }
private:
    ParameterType* value_;

};

template<class ParameterType>
class AttributeEnum : public AttributeAccessor<ParameterType> {
public:
    /// Construct.
    AttributeEnum(ParameterType* value, const std::vector<String>& names, ParameterType defaultValue) 
        : AttributeAccessor<ParameterType>(defaultValue)
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

    void SerializeInBlock(Se::Archive& archive, const String& name) override {
        SerializeValue(archive, name, *value_);
    }
private:
    ParameterType* value_;
    const std::vector<String>& names_;
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

    void SerializeInBlock(Se::Archive& archive, const String& name) override {
        ParameterType value = getFunction_();
        SerializeValue(archive, name, value);

        if (archive.IsInput())
            setFunction_(value);
    }

    bool IsDefault() const override {
        return this->defaultValue_ == getFunction_();
    }

private:
    /// Get functor.
    TGetFunction getFunction_;
    /// Set functor.
    TSetFunction setFunction_;
};

template<class T>
class Reflected;

class Attributes
{

public:
    Attributes() {}

    template<class T>
    void Register(const String& name, T* value, T defaultValue = T())
    {
        auto ptr = new AttributeValue<T>(value, defaultValue);
        values_[name] = std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr));
        // auto ptr = std::make_shared<AttributeValue<T>>(value, defaultValue);
        // values_[name] = std::dynamic_pointer_cast<AttributeEmpty>(ptr);
        ptr->ToDefault();
    }

    template<class T, class ObjectType, class TGetFunction, class TSetFunction>
    void Register(const String& name, ObjectType* object, TGetFunction getFunction, TSetFunction setFunction, T defaultValue = T())
    {
        auto funcGet = std::bind(getFunction, object);
        auto funcSet = std::bind(setFunction, object, std::placeholders::_1);
        auto ptr = new AttributeAccessorImpl<T, decltype(funcGet), decltype(funcSet)>(funcGet, funcSet, defaultValue);
        values_[name] = std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr));
        ptr->ToDefault();
    }

    std::shared_ptr<AttributeEmpty> FindAttribute(const String& name) {
        auto attr = values_.find(name);
        if (attr == values_.end())
        {
            SE_LOG_ERROR("Object has no attribute : {}", name);
            return nullptr;
        }
        return attr->second;
    }

    template<class T>
    T Get(const String& name)
    {
        auto attr = FindAttribute(name);

        if (!attr)
            return {};

        T value{0};
        reinterpret_cast<AttributeValue<T>*>(attr.get())->Get(&value);
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

    virtual void SerializeInBlock(Archive& archive) 
    {
        for  (auto& attr : values_)
            attr.second->SerializeInBlock(archive, attr.first);
    }

protected:
    std::unordered_map<String, std::shared_ptr<AttributeEmpty>> values_;
    inline static std::unordered_map<String, std::shared_ptr<AttributeEmpty>> staticValues_{};
};

class ReflectedObject;

template<class T>
class Reflected
{

public:

    using ReflectedType = T;

    Reflected()
        : type_(ToStringTypeId<T>())
    {
        Reflected* ptr = this;
        object_ = reinterpret_cast<T*>(ptr);
        //baseType_ =ToStringTypeId<decltype(*object_)>(); //  GetTypeOrig();
        //attributes_; = Attributes(object_);
        ptr->initAttributes();
    }

    virtual void SerializeInBlock(Archive& archive)
    {
        SerializeValue(archive, "Attributes", attributes_);
        //SE_LOG_ERROR("{} is not reflected object. Need to override method: void SerializeInBlock(Archive& archive)", type_);
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

    virtual void RegisterAttributes(Attributes& attributes)
    {
        SE_LOG_WARNING("{} attributes is empty", type_);
    }

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
        //auto attributes = Attributes(object_);
        newObject->initAttributes();
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
        RegisterAttributes(attributes_);
    }

protected:
    
    
    String type_;
    String baseType_;

    
private:
    T* object_;
    Attributes attributes_;
};

class ReflectedObject : public Reflected<ReflectedObject> {

public:

};

class ReflectedManager 
{
public:
    template<class T>
    static void Register()
    {
        registered_[ToStringTypeId<T>()] = 
            reinterpret_cast<std::shared_ptr<ReflectedObject>(*)()>(&T::Create);
    }

    static std::shared_ptr<ReflectedObject> Create(const String& type)
    {
        if (registered_.find(type) == registered_.end()) {
            // SE_LOG_WARNING("Type \"{}\" is not registered", type);
            return registered_[ReflectedObject::GetStaticType()]();
        }

        return registered_[type]();
    }

    template<class T>
    static std::shared_ptr<T> Create()
    {   
        //return std::dynamic_pointer_cast<T>(registered_[T::GetStaticType()]());
        return reinterpret_cast<std::shared_ptr<T>(*)()>(registered_[T::GetStaticType()])();
    }

    static std::unordered_map<String, std::shared_ptr<ReflectedObject>(*)()> registered_;
};

inline std::unordered_map<String, std::shared_ptr<ReflectedObject>(*)()> ReflectedManager::registered_ = {
    {ReflectedObject::GetStaticType(), &ReflectedObject::Create}
};

}