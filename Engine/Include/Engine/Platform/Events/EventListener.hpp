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

#include "Engine/Core/Macro.hpp"
#include "Engine/Platform/Window.hpp"

namespace Engine::Platform::Events
{
  class EventListener
  {
    DISALLOW_COPY( EventListener );

  public:
    EventListener() = default;
    EventListener( Window & window, Window::EventCallback callback );
    ~EventListener();

    EventListener( EventListener && other ) noexcept;
    EventListener & operator=( EventListener && other ) noexcept;

    [[nodiscard]] bool IsValid() const;

    void Remove();

  private:
    Window * m_pWindow;
    u8       m_Id;
  };
} // namespace Engine::Platform::Events
