#include <cmath>

#include <Engine/Utility/Logger.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include "Game/Application.hpp"

namespace Game
{
  Application::Application()
    : m_TotalTime( 0.0f )
  {
  }

  void Application::Init()
  {
    GetRenderer().Clear( 0.5f, 0.5f, 0.5f );
  }

  void Application::Update( const f32 deltaTime )
  {
    m_TotalTime += deltaTime;

    const f32 Red   = 0.2f + 0.1f + std::sin( m_TotalTime );
    const f32 Green = 0.1f + 0.1f * std::sin( m_TotalTime * 1.3f );
    const f32 Blue  = 0.3f + 0.1f * std::sin( m_TotalTime * 0.7f );

    GetRenderer().Clear( Red, Green, Blue );
  }

  void Application::Draw()
  {
  }

  void Application::Shutdown()
  {
    LOG_INFO( "Game shutdown" );
  }

  void Application::OnEvent( const Engine::Platform::WindowEvent & event )
  {
    using namespace Engine::Platform;

    if ( std::holds_alternative<KeyPressedEvent>( event ) )
    {
      const auto & [ m_Key, m_RepeatCount ] =
        std::get<KeyPressedEvent>( event );

      if ( m_Key == KeyCode::m_Escape )
      {
        Close();
      }
    }

    if ( std::holds_alternative<WindowResizeEvent>( event ) )
    {
      const auto & [ m_Width, m_Height ] = std::get<WindowResizeEvent>( event );
      LOG_INFO( "Window resized to {}x{}", m_Width, m_Height );
    }
  }
} // namespace Game
