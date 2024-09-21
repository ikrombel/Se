#include <Se/String.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#include <functional>

namespace Se {

/// Abstract base class for invoking attribute accessors.
template<class ParameterType>
class AttributeAccessor {
public:
    /// Construct.
    AttributeAccessor() = default;
    /// Get the attribute.
    virtual void Get(ParameterType* dest) const = 0;
    /// Set the attribute.
    virtual void Set(const ParameterType& src) = 0;

protected:
    /// @brief TODO
    ParameterType defaultValue_;
};

struct AttributeEmptyValue {};
class AttributeEmpty : public AttributeAccessor<AttributeEmptyValue>
{
    /// Construct.
    AttributeEmpty() : AttributeAccessor() {}
    /// Get the attribute.
    void Get(AttributeEmptyValue* dest) const override {};
    /// Set the attribute.
    void Set(const AttributeEmptyValue& src) override {};
};

template<class ParameterType>
class AttributeValue : public AttributeAccessor<ParameterType> {
public:
    /// Construct.
    AttributeValue(ParameterType* value) 
        : AttributeAccessor<ParameterType>()
        , value_(*value)
    {

    }
    /// Get the attribute.
    void Get(ParameterType* dest) const override {
        *dest = value_;
    };
    /// Set the attribute.
    void Set(const ParameterType& src) override {
        value_ = src;
    };
private:
    ParameterType value_;

};

/// Template implementation of the variant attribute accessor.
template </*class TClassType, */class ParameterType, class TGetFunction, class TSetFunction>
class AttributeAccessorImpl : public AttributeAccessor<ParameterType>
{
public:
    /// Construct.
    AttributeAccessorImpl(/*TClassType* ptr,*/ TGetFunction getFunction, TSetFunction setFunction) 
    : AttributeAccessor<ParameterType>()
    //, ptr_(ptr)
    , getFunction_(getFunction)
    , setFunction_(setFunction) {}

    /// Invoke getter function.
    void Get(ParameterType* value) const override
    {
        // assert(ptr);
        // const ParameterType classPtr = static_cast<const TClassType*>(ptr);
        *value = getFunction_();
    }

    /// Invoke setter function.
    void Set(const ParameterType& value) override
    {
        // assert(ptr);
        // auto classPtr = static_cast<TClassType*>(ptr);
        setFunction_(value);
    }

private:
    /// Get functor.
    TGetFunction getFunction_;
    /// Set functor.
    TSetFunction setFunction_;
};

template<class ObjectType>
class Attributes
{
public:
    Attributes() {}

    Attributes(ObjectType* obj) : object_(obj) {
        
    }

    template<class T>
    void Register(const String& name, T* value)
    {
        auto ptr = new AttributeValue<T>(value);
        values_[name] = std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr));
        //values_[name] = std::dynamic_pointer_cast<AttributeEmpty>(std::make_shared<AttributeValue<T>>(value));
    }

    template<class T, class TGetFunction, class TSetFunction>
    void Register(const String& name, TGetFunction getFunction, TSetFunction setFunction/*, const T& defaultValue = T()*/)
    {
//        auto ptr = new AttributeAccessorImpl<T, TGetFunction, TSetFunction>(getFunction, setFunction); 
        auto funcGet = std::bind(getFunction, object_);
        auto funcSet = std::bind(setFunction, object_, std::placeholders::_1);
        auto ptr = new AttributeAccessorImpl<T, decltype(funcGet), decltype(funcSet)>(funcGet, funcSet);
        values_[name] = std::shared_ptr<AttributeEmpty>(reinterpret_cast<AttributeEmpty*>(ptr));
    }

    template<class T>
    T Get(const String& name)
    {
        T value{0};
        //std::dynamic_pointer_cast<AttributeValue<T>>(values_[name])->Get(&value);
        auto casted = reinterpret_cast<AttributeValue<T>*>(values_[name].get());
        casted->Get(&value);
        return value;
    }

    template<class T>
    void Set(const String& name, const T& value)
    {
        //std::dynamic_pointer_cast<AttributeValue<T>>(values_[name])->Set(value);
        auto casted = reinterpret_cast<AttributeValue<T>*>(values_[name].get());
        casted->Set(value);
    }

    

protected:
    std::unordered_map<String, std::shared_ptr<AttributeEmpty>> values_;
    inline static std::unordered_map<String, std::shared_ptr<AttributeEmpty>> staticValues_{};
    ObjectType* object_;
};

template<class T>
class Reflected
{
public:

    Reflected() {
        Reflected* ptr = this;
        object_ = reinterpret_cast<T*>(ptr);
        attributes_ = Attributes(object_);
        RegisterAttributes(object_, attributes_);
    }

    virtual void SerializeInBlock(Archive& archive)
    {
        SE_LOG_ERROR("{} is not reflected object. Need to override method: void SerializeInBlock(Archive& archive)", type_);
    }

    virtual void RegisterAttributes(T* object, Attributes<T>& attributes)
    {
        SE_LOG_WARNING("{} attributes is empty", type_);
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
    static void Register(std::function<void(T* ,Attributes<T>&)> attributesFunc = [](T*, Attributes<T>&){})
    {
        registered_[T::GetStaticType()] = 
            reinterpret_cast<std::shared_ptr<ReflectedObject>(*)()>(&T::Create);

        //Attributes attr;
        //attributesFunc(attr);
        //T::RegisterAttributes(attr);
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