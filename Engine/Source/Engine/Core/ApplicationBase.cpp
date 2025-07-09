#include <chrono>

#include "Engine/Platform/WindowFactory.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Core/ApplicationBase.hpp"

namespace Engine::Core
{
  ApplicationBase::ApplicationBase()
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

    while ( m_IsRunning && !m_Window->ShouldClose() )
    {
      auto       time  = std::chrono::high_resolution_clock::now();
      const auto Delta = std::chrono::duration<f32>( time - last ).count();
      last             = time;

      m_Window->PollEvents();
      Update( Delta );

      m_Renderer->BeginDraw();
      if ( m_Renderer->IsFrameInProgress() )
      {
        Draw();
        m_Renderer->EndDraw();
      }
    }

    Shutdown();
  }

  void ApplicationBase::Close()
  {
    m_IsRunning = false;
  }

  Platform::IWindow & ApplicationBase::GetWindow() const
  {
    return *m_Window;
  }

  Renderer::Renderer & ApplicationBase::GetRenderer() const
  {
    return *m_Renderer;
  }

  void ApplicationBase::InternalInit()
  {
    try
    {
      const Platform::WindowProps Props = {};

      m_Window = Platform::IWindow::Create( Props );
      m_Window->SetEventCallback(
        [ this ]( const Platform::WindowEvent & EVENT )
        { HandleEvent( EVENT ); } );

      m_Renderer = std::make_unique<Renderer::Renderer>( *m_Window );
    }
    catch ( const std::exception & E )
    {
      LOG_FATAL( "Failed to initialize engine: {}", E.what() );
    }

    m_IsRunning = true;
    LOG_INFO( "Successfully initialized engine" );
  }

  void ApplicationBase::InternalShutdown()
  {
    if ( m_Renderer )
    {
      m_Renderer.reset();
    }

    if ( m_Window )
    {
      m_Window.reset();
    }

    LOG_INFO( "Successfully shutdown engine" );
  }

  void ApplicationBase::HandleEvent( const Platform::WindowEvent & event )
  {
    if ( std::holds_alternative<Platform::WindowCloseEvent>( event ) )
    {
      Close();
    }

    OnEvent( event );
  }
} // namespace Engine::Core
