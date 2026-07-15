#pragma once

#include <Se/Console.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#include <functional>
#include <memory>
#include <unordered_map>


namespace Se
{

class Extension;

template<typename T = Extension>
class Module;

class Extension
{
public:

    Extension(const std::string& name)
    {
        name_ = name;
    }

    virtual ~Extension() //= default;
    {
        SE_LOG_DEBUG("Extension {} destroyed", name_);
    }

    // void SetModule(Module<Extension>* module) 
    // {
    //     module_ = module;
    // }

    /// Get extension name.
    const std::string& GetName() const { return name_; }
    /// Check whether the extension is enabled.
    bool IsEnabled() const { return enabled_; }
    /// Set enabled state.
    void SetEnabled(bool enable) { enabled_ = enable; }

    // bool CheckDependencies()
    // {
    //     for (auto& dep : dependencies_)
    //     {
    //         if (!dep->IsEnabled())
    //             return false;
    //     }
    //     return true;
    // }

    virtual void SerializeInBlock(Archive& archive)
    {
        SerializeValue(archive, "Name", name_);
        SerializeValue(archive, "Enabled", enabled_);
        SerializeValue(archive, "Dependencies", dependencies_);

    }
    

private:
    //Module<Extension>* module_ = nullptr;
    /// 
    std::string name_ = "";
    /// 
    bool enabled_ = true;
    /// required dependies
    std::vector<std::string> dependencies_;

};

template<typename T>
class Module
{
public:
    Module(const std::string& extBaseName) {
        extBaseName_ = extBaseName;
    }

    ~Module() {
        extensionFactory_.clear();
    }

    virtual void Init() {}

    template<typename U0, typename U1>
    struct Reg {
        Reg(U0* module, const char* typeName) { 
            module->template Register<U1>(typeName);
        }
    };

    std::string GetBaseName() 
    {
        return extBaseName_;
    }

protected:
    std::unordered_map<std::string, std::shared_ptr<T>> extensionFactory_;
    std::string extBaseName_ = "";

private:

    template<typename U0, typename U1>
    friend struct Reg;
  
    template<class U>
    void Register(const std::string& name)
    {
        auto& factory = this->extensionFactory_;

        auto resIt = factory.find(name);

        if (resIt != factory.end())
        {
            SE_LOG_WARNING("{} {} already registered", extBaseName_, name);
            return;
        }

        factory[name] = std::make_shared<U>(); //(this, name);
    }
};

class ModuleManager
{
public:
    // template<typename T>
    // T* GetModule() 
    // {
    //     std::string name = typeid(T).name();
    //     for (auto module : modules_)
    //     {
    //         if (module.first == name)
    //             return reinterpret_cast<T*>(it->second);
    //     }

    //     SE_LOG_ERROR("Module {} not found", name);
    //     return nullptr;
    // }

    template<typename T>
    void AddModule() 
    {
        modules_.push_back({typeid(T).name(), reinterpret_cast<Module<Extension>*>(T::Get())});
    }

    ~ModuleManager() 
    {
        Destroy();
    }

    void Destroy() 
    {
        String dbgLStr = "Destroyed modules:";
        for (auto it = modules_.rbegin(); it != modules_.rend(); ++it) {
            dbgLStr += " " + it->second->GetBaseName();
            delete it->second;
        }
        modules_.clear();
        SE_LOG_DEBUG(dbgLStr);
    }
    
protected:
    std::vector<std::pair<std::string, Module<Extension>*>> modules_;
};

// inline void SerializeValue(Archive& archive, const char* name, StringHash& value) {
//     archive.Serialize(name, value.MutableValue());
// }

} // namespace Se


#define SE_MODULE(moduleName) \
public: \
    moduleName() : Module(#moduleName) { Init(); } \
    static moduleName* Get() \
    { \
        static moduleName* ptr = nullptr; \
        if (!ptr) \
            ptr = new moduleName(); \
        return ptr; \
    }

#define SE_REGISTER_EXTENSION(moduleName, typeName) \
namespace RegScope { \
    const moduleName::Reg<moduleName, typeName> reg_##typeName(moduleName::Get(), #typeName); }