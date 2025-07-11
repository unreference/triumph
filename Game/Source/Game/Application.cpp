#include <cmath>

#include <Engine/Utility/Logger.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include "Game/Application.hpp"

namespace Game
{
  Application::Application()
    : m_EscapeKeyPressedListener()
    , m_KeyPressedListener()
    , m_MouseButtonPressedListener()
    , m_TotalTime( 0 )
  {
  }
  void Application::Init()
  {
    GetRenderer().Clear( 0.5f, 0.5f, 0.5f );
    SetupGameEventListeners();
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

  void Application::SetupGameEventListeners()
  {
    using namespace Engine::Platform::Events;

    m_EscapeKeyPressedListener = KeyPressedListener(
      GetWindow(),
      [ this ]( const KeyPressedEvent & event )
      {
        if ( event.m_Key == KeyCode::m_Escape )
        {
          LOG_INFO( "Escape pressed.  Closing application" );
          Close();
        }
      } );

    m_KeyPressedListener = KeyPressedListener(
      GetWindow(),
      [ this ]( const KeyPressedEvent & event )
      {
        LOG_INFO( "Key pressed: {} (repeated: {}",
                  static_cast<i32>( event.m_Key ), event.m_RepeatCount );
      } );

    m_MouseButtonPressedListener = MouseButtonPressedListener(
      GetWindow(),
      [ this ]( const MouseButtonPressedEvent & event )
      {
        LOG_INFO( "Mouse button pressed: {}",
                  static_cast<i32>( event.m_Button ) );
      } );
  }

} // namespace Game
