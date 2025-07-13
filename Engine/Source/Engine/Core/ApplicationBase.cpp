/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include <chrono>

#include "Engine/Platform/WindowFactory.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Core/ApplicationBase.hpp"

namespace Engine::Core
{
  ApplicationBase::ApplicationBase()
    : m_CloseListener()
    , m_ResizeListener()
    , m_IsRunning( false )
    , m_LastFrameTime( 0.0f )
  {
    InternalInit();
  }

  ApplicationBase::~ApplicationBase()
  {
    InternalShutdown();
  }

  void ApplicationBase::Run()
  {
    Init();

    auto last = std::chrono::high_resolution_clock::now();

    while ( m_IsRunning && !m_pWindow->ShouldClose() )
    {
      auto       time  = std::chrono::high_resolution_clock::now();
      const auto Delta = std::chrono::duration<f32>( time - last ).count();
      last             = time;

      m_pWindow->PollEvents();
      Update( Delta );

      m_pRenderer->BeginDraw();
      if ( m_pRenderer->IsFrameInProgress() )
      {
        Draw();
        m_pRenderer->EndDraw();
      }
    }

    Shutdown();
  }

  void ApplicationBase::Close()
  {
    m_IsRunning = false;
  }

  Platform::Window & ApplicationBase::GetWindow() const
  {
    return *m_pWindow;
  }

  Renderer::Renderer & ApplicationBase::GetRenderer() const
  {
    return *m_pRenderer;
  }

  void ApplicationBase::InternalInit()
  {
    try
    {
      const Platform::WindowProps Props = {};
      m_pWindow                         = Platform::Window::Create( Props );
      m_pRenderer = std::make_unique<Renderer::Renderer>( *m_pWindow );
      SetupEngineEventListeners();
    }
    catch ( const std::exception & E )
    {
      LOG_FATAL( "Failed to initialize engine: {}", E.what() );
    }

    m_IsRunning = true;
  }

  void ApplicationBase::InternalShutdown()
  {
    if ( m_CloseListener.IsValid() )
    {
      m_CloseListener.Remove();
    }

    if ( m_ResizeListener.IsValid() )
    {
      m_ResizeListener.Remove();
    }

    if ( m_pRenderer )
    {
      m_pRenderer.reset();
    }

    if ( m_pWindow )
    {
      m_pWindow.reset();
    }
  }

  void ApplicationBase::SetupEngineEventListeners()
  {
    using namespace Platform::Events;

    m_CloseListener = WindowCloseListener(
      *m_pWindow, [ this ]( const WindowCloseEvent & ) { Close(); } );

    m_ResizeListener = WindowResizeListener(
      *m_pWindow, [ this ]( const WindowResizeEvent & event )
      { m_pRenderer->Resize( event.m_Width, event.m_Height ); } );
  }
} // namespace Engine::Core
