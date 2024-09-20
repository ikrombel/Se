#include <Se/String.hpp>
#include <SeArc/ArchiveSerialization.hpp>


namespace Se {

template<class T>
class Reflected
{
public:
    virtual void SerializeInBlock(Archive& archive)
    {
        SE_LOG_ERROR("{} is not reflected object. Need to override method: void SerializeInBlock(Archive& archive)", type_);
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