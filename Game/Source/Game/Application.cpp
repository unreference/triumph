/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

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
    , m_TotalTime( 0.0f )
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
  }

  void Application::Draw()
  {
  }

  void Application::Shutdown()
  {
  }

  void Application::SetupGameEventListeners()
  {
    using namespace Engine::Platform::Events;

    m_EscapeKeyPressedListener =
      KeyPressedListener( GetWindow(),
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
        LOG_INFO( "Key pressed: {} (repeated: {}", static_cast<i32>( event.m_Key ),
                  event.m_RepeatCount );
      } );

    m_MouseButtonPressedListener = MouseButtonPressedListener(
      GetWindow(),
      [ this ]( const MouseButtonPressedEvent & event )
      {
        LOG_INFO( "Mouse button pressed: {}", static_cast<i32>( event.m_Button ) );
      } );
  }
} // namespace Game
