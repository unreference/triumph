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

    bool IsValid() const;

    void Remove();

  private:
    Window * m_Window;
    u64      m_Id;
  };
} // namespace Engine::Platform::Events
