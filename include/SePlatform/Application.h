#pragma once

#include <SePlatform/Platform.hpp>
#include <SeVFS/MountedDirectory.h>


// #if ANDROID
// #  if __has_include("SePlatformAndroid.h")
// #    include <SePlatformAndroid.h>
// #    define SE_APPLICATION_RENDERER 1
// #  endif
// #else
// #  if SE_GLFW
// #    include <SePlatformGLFW.h>
// #    define SE_APPLICATION_RENDERER 1
// #  endif
// #endif

namespace Se
{

class Archive;
class Application;

class Config
{
public:

    Config(Application* app)
        : app_(app)
    {

    }

    bool Save();

    bool Load();

    Application* app_;
};


class Application
{
public:

    Application(const String name) {
        appName_ = name;
        app_ = this;
    }

    ~Application();

    virtual void Init();

    void Run();

//     void InitRenderer()
//     {
// #if SE_APPLICATION_RENDERER
//         Window::onCreate = [this]() -> std::shared_ptr<Window>
//         {
//         #if ANDROID
//             return std::make_shared<WindowAndroid>(appName_); 
//         #else
//             return std::make_shared<WindowGLFW>(appName_); 
//         #endif
//         };
// #endif
//     }

    virtual void SerializeInBlock(Archive& archive);

    //MountedDirectory* GetDirCfg() { return mDirGfg_.get(); }

    String GetProgramPrefDir() const {
        return pathProgramPref;
    }

    inline static Application* Get() {
        if (!app_) {
            SE_LOG_ERROR("Application initialization incorrect");
            assert(0);
        }

        return app_;
    }


protected:

    inline static Application* app_;

    std::shared_ptr<Se::Window> window_;

    String appName_{"SEApplication"};

    std::shared_ptr<Config> cfg_;

    std::shared_ptr<MountedDirectory> mDirGfg_;

    String pathProgramPref;
};

}