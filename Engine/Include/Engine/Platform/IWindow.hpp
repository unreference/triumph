#pragma once

#include <functional>

#include "Engine/Core/Types.hpp"

#include "WindowEvents.hpp"

namespace Engine::Platform
{
  struct WindowProps
  {
    std::string m_Title  = "Triumph";
    u32         m_Width  = 1280;
    u32         m_Height = 720;

    // #NOTE: Possible flags candidate
    bool m_IsVsynced    = false;
    bool m_IsResizable  = true;
    bool m_IsFullScreen = false;
    bool m_IsDecorated  = true;
  };

  class IWindow
  {
  public:
    using EventCallback = std::function<void( const WindowEvent & )>;

    virtual ~IWindow() = default;

    static std::unique_ptr<IWindow> Create( const WindowProps & props );

    [[nodiscard]] virtual Result<vk::SurfaceKHR>
    CreateSurface( vk::Instance instance ) const = 0;

    virtual void               PollEvents()        = 0;
    virtual void               SwapBuffers()       = 0;
    [[nodiscard]] virtual bool ShouldClose() const = 0;
    virtual void               Close()             = 0;

    [[nodiscard]] virtual std::vector<const char *>
                                      GetRequiredExtensions() const = 0;
    [[nodiscard]] virtual std::string GetTitle() const              = 0;
    [[nodiscard]] virtual u32         GetWidth() const              = 0;
    [[nodiscard]] virtual u32         GetHeight() const             = 0;
    [[nodiscard]] virtual bool        IsVsynced() const             = 0;
    [[nodiscard]] virtual bool        IsFullScreen() const          = 0;

    [[nodiscard]] virtual void * GetNativeHandle() const = 0;

    virtual void SetTitle( const std::string & title )      = 0;
    virtual void SetSize( u32 width, u32 height )           = 0;
    virtual void SetVsync( bool isEnabled )                 = 0;
    virtual void SetFullScreen( bool isEnabled )            = 0;
    virtual void SetEventCallback( EventCallback callback ) = 0;
  };
} // namespace Engine::Platform
