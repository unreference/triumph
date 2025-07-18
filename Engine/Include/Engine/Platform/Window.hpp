/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Types.hpp"
#include "Engine/Core/Result.hpp"
#include "Events/WindowEvents.hpp"

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

  class Window
  {
  public:
    using EventCallback = std::function<void( const Events::WindowEvent & )>;

    Window();
    virtual ~Window() = default;

    static std::unique_ptr<Window> Create( const WindowProps & props );

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
    [[nodiscard]] virtual void *      GetNativeHandle() const       = 0;

    virtual void SetTitle( const std::string & title ) = 0;
    virtual void SetSize( u32 width, u32 height )      = 0;
    virtual void SetVsync( bool isEnabled )            = 0;
    virtual void SetFullScreen( bool isEnabled )       = 0;

    virtual u8   AddEventListener( EventCallback callback ) = 0;
    virtual bool RemoveEventListener( u8 id )               = 0;
    virtual void ClearEventListeners()                      = 0;

  protected:
    void DispatchEvent( const Events::WindowEvent & event ) const;

  private:
    struct EventListener
    {
      u8            m_Id = 0;
      EventCallback m_Callback;
    };

    std::vector<EventListener> m_EventListeners;
    u8                         m_NextListenerId;
  };

} // namespace Engine::Platform
