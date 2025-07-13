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

#include "Engine/Platform/Window.hpp"

namespace Engine::Platform::Events
{
  template <typename Event> class TypedEventListener
  {
    DISALLOW_COPY( TypedEventListener );

  public:
    using TypedCallback = std::function<void( const Event & )>;

    TypedEventListener() = default;

    TypedEventListener( Window & window, TypedCallback callback )
      : m_Listener( window,
                    [ callback ]( const WindowEvent & event )
                    {
                      if ( std::holds_alternative<Event>( event ) )
                      {
                        callback( std::get<Event>( event ) );
                      }
                    } )
    {
    }

    TypedEventListener & operator=( TypedEventListener && other ) noexcept
    {
      m_Listener = std::move( other.m_Listener );
      return *this;
    }

    [[nodiscard]] bool IsValid() const
    {
      return m_Listener.IsValid();
    }

    void Remove()
    {
      m_Listener.Remove();
    }

  private:
    EventListener m_Listener;
  };

  using WindowCloseListener         = TypedEventListener<WindowCloseEvent>;
  using WindowResizeListener        = TypedEventListener<WindowResizeEvent>;
  using WindowSetFocusListener      = TypedEventListener<WindowSetFocusEvent>;
  using WindowKillFocusListener     = TypedEventListener<WindowKillFocusEvent>;
  using WindowMovedListener         = TypedEventListener<WindowMovedEvent>;
  using KeyPressedListener          = TypedEventListener<KeyPressedEvent>;
  using KeyReleasedListener         = TypedEventListener<KeyReleasedEvent>;
  using KeyTypedListener            = TypedEventListener<KeyTypedEvent>;
  using MouseButtonPressedListener  = TypedEventListener<MouseButtonPressedEvent>;
  using MouseButtonReleasedListener = TypedEventListener<MouseButtonReleasedEvent>;
  using MouseMovedListener          = TypedEventListener<MouseMovedEvent>;
  using MouseScrolledListener       = TypedEventListener<MouseScrolledEvent>;
} // namespace Engine::Platform::Events
