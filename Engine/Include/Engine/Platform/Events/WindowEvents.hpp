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

#include <variant>

#include "Engine/Core/Types.hpp"

namespace Engine::Platform::Events
{
  enum class EventType : u8
  {
    m_WindowClose,
    m_WindowResize,
    m_WindowSetFocus,
    m_WindowKillFocus,
    m_WindowMoved,
    m_KeyPressed,
    m_KeyReleased,
    m_KeyTyped,
    m_MouseButtonPressed,
    m_MouseButtonReleased,
    m_MouseMoved,
    m_MouseScrolled,
  };

  enum class KeyCode : char // u32 for UNICODE?
  {
    m_Tab   = 0x9,
    m_Enter = 0xD,
    m_Shift = 0x10,
    m_Control,
    m_Alt,

    m_Escape = 0x1B,
    m_Space  = 0x20,

    m_0 = 0x30,
    m_1,
    m_2,
    m_3,
    m_4,
    m_5,
    m_6,
    m_7,
    m_8,
    m_9,

    m_A = 0x41,
    m_B,
    m_C,
    m_D,
    m_E,
    m_F,
    m_G,
    m_H,
    m_I,
    m_J,
    m_K,
    m_L,
    m_M,
    m_N,
    m_O,
    m_P,
    m_Q,
    m_R,
    m_S,
    m_T,
    m_U,
    m_V,
    m_W,
    m_X,
    m_Y,
    m_Z,

    m_Num0 = 0x60,
    m_Num1,
    m_Num2,
    m_Num3,
    m_Num4,
    m_Num5,
    m_Num6,
    m_Num7,
    m_Num8,
    m_Num9,

    m_F1 = 0x70,
    m_F2,
    m_F3,
    m_F4,
    m_F5,
    m_F6,
    m_F7,
    m_F8,
    m_F9,
    m_F10,
    m_F11,
    m_F12,

    m_Left = 0x25,
    m_Up,
    m_Right,
    m_Down,
  };

  enum class MouseButton : u8
  {
    m_Left,
    m_Right,
    m_Middle,
    m_Button4,
    m_Button5,
    m_Button6,
    m_Button7,
    m_Button8,
  };

  struct WindowCloseEvent
  {
    static constexpr auto Type = EventType::m_WindowClose;
  };

  struct WindowResizeEvent
  {
    static constexpr auto Type     = EventType::m_WindowResize;
    u32                   m_Width  = 0;
    u32                   m_Height = 0;
  };

  struct WindowSetFocusEvent
  {
    static constexpr auto Type = EventType::m_WindowSetFocus;
  };

  struct WindowKillFocusEvent
  {
    static constexpr auto Type = EventType::m_WindowKillFocus;
  };

  struct WindowMovedEvent
  {
    static constexpr auto Type = EventType::m_WindowMoved;
    i32                   m_X  = 0;
    i32                   m_Y  = 0;
  };

  struct KeyPressedEvent
  {
    static constexpr auto Type          = EventType::m_KeyPressed;
    KeyCode               m_Key         = {};
    u8                    m_RepeatCount = 0;
  };

  struct KeyReleasedEvent
  {
    static constexpr auto Type  = EventType::m_KeyReleased;
    KeyCode               m_Key = {};
  };

  struct KeyTypedEvent
  {
    static constexpr auto Type        = EventType::m_KeyTyped;
    char                  m_Character = {};
  };

  struct MouseButtonPressedEvent
  {
    static constexpr auto Type     = EventType::m_MouseButtonPressed;
    MouseButton           m_Button = {};
  };

  struct MouseButtonReleasedEvent
  {
    static constexpr auto Type     = EventType::m_MouseButtonReleased;
    MouseButton           m_Button = {};
  };

  struct MouseMovedEvent
  {
    static constexpr auto Type = EventType::m_MouseMoved;
    f32                   m_X  = 0.0f;
    f32                   m_Y  = 0.0f;
  };

  struct MouseScrolledEvent
  {
    static constexpr auto Type      = EventType::m_MouseScrolled;
    f32                   m_XOffset = 0.0f;
    f32                   m_YOffset = 0.0f;
  };

  using WindowEvent =
    std::variant<WindowCloseEvent, WindowResizeEvent, WindowSetFocusEvent,
                 WindowKillFocusEvent, WindowMovedEvent, KeyPressedEvent,
                 KeyReleasedEvent, KeyTypedEvent, MouseButtonPressedEvent,
                 MouseButtonReleasedEvent, MouseMovedEvent, MouseScrolledEvent>;

  inline EventType GetEventType( const WindowEvent & event )
  {
    return std::visit( []( const auto & e ) { return e.Type; }, event );
  }
} // namespace Engine::Platform::Events
