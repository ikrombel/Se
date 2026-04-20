#pragma once

#include <string>

#include <functional>
#include <vector>
#include <memory>

#include <Se/IO/FileSystem.h>
#include <SeMath/ArchiveMath.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#ifdef __ANDROID__
struct AAsset;
#endif

namespace Se {

enum RenderBackend
{
    OpenGL,
    Vulkan,
    External
};

struct BackendAPI {
    BackendAPI() {}

    std::string name_;
    std::function<void()> onInitHint;
    std::function<void()> onMakeContext;
    std::function<void()> onRenderer;
};

class Renderer;


class Window {

public:
    inline static Signal<bool&> onWindowClose;

    inline static std::function<void(void*)> onImGuiInit;
    inline static std::function<void()> onImGuiNewFrame;
    inline static std::function<void()> onImGuiDestroy;

    inline static std::function<std::shared_ptr<Window>()> onCreate;

    Window(const std::string& title) : title_(title) {}

    virtual void Init(Se::RenderBackend gapi) {}

    virtual bool WindowShouldClose() { return true; }

    virtual void Render() {}
    ///
    virtual void SetWindowSize(int width, int height) {}
    /// 
    IntVector2 GetWindowSize() const { return windowSize_; }

    virtual void Close() {};

    virtual void SetWindowPosition(int x, int y) {}

    static std::vector<BackendAPI*> backends_;

    static BackendAPI* RegisterBackendAPI(const std::string& name) {
        BackendAPI* gapi = new BackendAPI();
        gapi->name_ = name;
        backends_.push_back(gapi);
        return gapi;
    }

    virtual std::vector<const char*> getVulkanInstanceExtensions() { return {}; }

    BackendAPI* backend_;

    virtual void* GetWindowPtr() { return nullptr; }

    virtual void SetWindowShouldClose(bool value) {}
    ///
    virtual IntVector2 GetFramebufferSize() { return IntVector2::ZERO; };

    virtual bool IsIconified() { return false; }

    virtual void Sleep(int mseconds) {};

    inline static std::shared_ptr<Window> Get() {
        if (!wnd_)
        {
            wnd_ = Window::onCreate();
        }

        return wnd_;
    }
    Renderer* GetRenderer() { return renderer_; }

    virtual void SerializeInBlock(Archive& archive) {
        // SerializeValue(archive, "title", title_);
#ifdef SE_DESKTOP
        SerializeOptionalValue(archive, "Size", windowSize_, IntVector2(1366, 768));
        SerializeOptionalValue(archive, "Position", windowPos_, IntVector2::ZERO);
        SerializeOptionalValue(archive, "Fullscreen", fullscreen_, true);
#endif
        SerializeValue(archive, "RenderBackend", gapi_);
    }

    virtual void ApplyConfigs() {};

    RenderBackend GetRenderBackend() const { return gapi_; }

#if ANDROID
    virtual bool FileExists(const String& fileName) const = 0;
    virtual bool DirExists(const String& pathName) const = 0;
    virtual bool ScanDirInternal(std::vector<String>& result, const String& path, const String& startPath,
            const String& filter, ScanFlags flags) const = 0;
    virtual bool ScanDirInternalTree(DirectoryNode& result, const String& path, const String& startPath,
            const String& filter, ScanFlags flags) const = 0;

    virtual AAsset* OpenAsset(const String& fileName) = 0; 
#endif


protected:
    std::string title_;

    inline static std::shared_ptr<Window> wnd_{nullptr};

    RenderBackend gapi_{RenderBackend::OpenGL};
    Renderer* renderer_;

    /// Window size
    IntVector2 windowSize_{1366, 768};
    /// Window position
    IntVector2 windowPos_{};
    ///
    bool fullscreen_{false};
};

}