#pragma once

#include <Se/Console.hpp>

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

    void SetModule(Module<Extension>* module) 
    {
        module_ = module;
    }

    private:
        Module<Extension>* module_ = nullptr;
        std::string name_ = "";
};

template<typename T>
class Module
{
public:
    Module(const std::string& extBaseName) {
        extBaseName_ = extBaseName;
    }

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



} // namespace Se


#define SE_MODULE(moduleName) \
public: \
    moduleName() : Module(#moduleName) {} \
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