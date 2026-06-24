#pragma once

#include <functional>
#include <memory>
#include <string>

#include <glm/vec2.hpp>

namespace Zenith
{
    class InputState;

    enum class GraphicsApi
    {
        None = 0,
        Bgfx
    };

    enum class BgfxRendererBackend
    {
        Auto = 0,
        Noop,
        Agc,
        Direct3D11,
        Direct3D12,
        Gnm,
        Metal,
        Nvn,
        OpenGL,
        OpenGLES,
        Vulkan,
    };

    enum class CursorMode
    {
        Normal = 0,
        Hidden,
        Disabled
    };

    struct WindowConfig
    {
        std::string title = "Zenith Engine - Dev";
        int width = 1280;
        int height = 720;
        bool fullscreen = false;
        bool vsync = true;
        bool resizable = true;
        GraphicsApi graphicsApi = GraphicsApi::Bgfx;
        BgfxRendererBackend bgfxBackend = BgfxRendererBackend::Vulkan;
    };

    enum class WindowEventType
    {
        CloseRequested = 0,
        Resized,
        FramebufferResized,
        FocusChanged
    };

    struct WindowEvent
    {
        WindowEventType type = WindowEventType::Resized;
        glm::ivec2 size{0, 0};
        bool focused = true;
    };

    using WindowEventCallback = std::function<void(const WindowEvent &)>;

    class WindowBackend
    {
    public:
        virtual ~WindowBackend() = default;

        virtual void pollEvents() = 0;
        virtual void swapBuffers() = 0;
        virtual bool shouldClose() const = 0;
        virtual void setShouldClose(bool close) = 0;
        virtual void setTitle(const std::string &title) = 0;
        virtual void setVSync(bool enabled) = 0;
        virtual glm::ivec2 getSize() const = 0;
        virtual glm::ivec2 getFramebufferSize() const = 0;
        virtual float getAspectRatio() const = 0;
        virtual void *getNativeHandle() const = 0;
        virtual void setCursorMode(CursorMode mode) = 0;
        virtual CursorMode getCursorMode() const = 0;
        virtual const char *getBackendName() const = 0;
        virtual GraphicsApi getGraphicsApi() const = 0;
    };

    std::unique_ptr<WindowBackend> createDefaultWindowBackend(
        const WindowConfig &config,
        InputState &input,
        WindowEventCallback eventCallback);

} // namespace Zenith
