#include <Se/String.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#include <functional>

namespace Se {


class AttributeInfo 
{
public:
    virtual String GetTypeName() const {
        return ""; }
};

/// Abstract base class for invoking attribute accessors.
template<class ParameterType>
class AttributeAccessor : public AttributeInfo {
public:
    /// Construct.
    AttributeAccessor(ParameterType defaultValue) : AttributeInfo() {
        defaultValue_ = defaultValue;
    };

    /// Get the attribute.
    virtual void Get(ParameterType* dest) const = 0;
    /// Set the attribute.
    virtual void Set(const ParameterType& src) = 0;

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

struct AttributeEmptyValue {}emptyValue;
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
       // *value_ = defaultValue;
        
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
private:
    ParameterType* value_;

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

template<class ObjectType>
class Attributes
{
    friend class Reflected<ObjectType>;
public:
    Attributes() {}

    Attributes(ObjectType* obj) : object_(obj) {
        
    }

    template<class T>
    void Register(const String& name, T* value, T defaultValue = T())
    {
        auto ptr = new AttributeValue<T>(value, defaultValue);
        values_[name] = std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr));
        ptr->ToDefault();
    }

    template<class T, class TGetFunction, class TSetFunction>
    void Register(const String& name, TGetFunction getFunction, TSetFunction setFunction, T defaultValue = T())
    {
        auto funcGet = std::bind(getFunction, object_);
        auto funcSet = std::bind(setFunction, object_, std::placeholders::_1);
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

        reinterpret_cast<AttributeValue<T>*>(attr.get())->Set(value);
    }

    

protected:
    std::unordered_map<String, std::shared_ptr<AttributeEmpty>> values_;
    inline static std::unordered_map<String, std::shared_ptr<AttributeEmpty>> staticValues_{};
    ObjectType* object_;
};

class ReflectedObject;

template<class T>
class Reflected
{

public:

    Reflected() {
        Reflected* ptr = this;
        object_ = reinterpret_cast<T*>(ptr);
        attributes_ = Attributes(object_);
        object_->RegisterAttributes(attributes_);
    }

    virtual void SerializeInBlock(Archive& archive)
    {
        SE_LOG_ERROR("{} is not reflected object. Need to override method: void SerializeInBlock(Archive& archive)", type_);
    }

    virtual void RegisterAttributes(Attributes<T>& attributes) 
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
        auto& attr = attributes_.values_[name];
        if (attr->GetTypeName() == "float" && ToStringTypeId<U>() == "double") //TODO simplify
            attributes_.Set(name, (float)value);
        else
            return attributes_.Set(name, value);
    } 

    static std::shared_ptr<T> Create() {
        return std::make_shared<T>();
    }

    String GetType() {
        return type_;
    }

    static String GetStaticType() {
        static String staticType{};
        if (!staticType.empty())
            return staticType;
        return ToStringTypeId<T>();
    }

protected:
    
    String type_ = ToStringTypeId<T>();

    Attributes<T> attributes_;
private:
    T* object_{dynamic_cast<T*>(this)};
};

class ReflectedObject : public Reflected<ReflectedObject> {};

class ReflectedManager 
{
public:
    template<class T>
    static void Register()
    {
        registered_[T::GetStaticType()] = 
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