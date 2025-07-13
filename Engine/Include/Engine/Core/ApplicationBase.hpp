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

#include <memory>

#include "Engine/Platform/Events/EventListener.hpp"
#include "Engine/Platform/Events/TypedEventListener.hpp"

namespace Engine::Renderer
{
  class Renderer;
} // namespace Engine::Renderer

namespace Engine::Core
{
  class ApplicationBase
  {
    DISALLOW_COPY( ApplicationBase );
    DISALLOW_MOVE( ApplicationBase );

  public:
    ApplicationBase();
    virtual ~ApplicationBase();

    void Run();
    void Close();

    [[nodiscard]] Platform::Window &   GetWindow() const;
    [[nodiscard]] Renderer::Renderer & GetRenderer() const;

  protected:
    virtual void Init()                  = 0;
    virtual void Update( f32 deltaTime ) = 0;
    virtual void Draw()                  = 0;
    virtual void Shutdown()              = 0;

  private:
    void InternalInit();
    void InternalShutdown();
    void SetupEngineEventListeners();

    std::unique_ptr<Platform::Window>   m_pWindow;
    std::unique_ptr<Renderer::Renderer> m_pRenderer;

    Platform::Events::WindowCloseListener  m_CloseListener;
    Platform::Events::WindowResizeListener m_ResizeListener;

    bool m_IsRunning;
    f32  m_LastFrameTime;
  };

  ApplicationBase * Create();

} // namespace Engine::Core
