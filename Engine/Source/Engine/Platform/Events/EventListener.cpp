#include "Engine/Utility/Logger.hpp"

#include "Engine/Platform/Events/EventListener.hpp"

namespace Engine::Platform::Events
{
  EventListener::EventListener( Window &              window,
                                Window::EventCallback callback )
    : m_Window( &window )
  {
    m_Id = m_Window->AddEventListener( std::move( callback ) );
    if ( m_Id == 0 )
    {
      LOG_FATAL( "Failed to add event listener!" );
      m_Window = nullptr;
    }
  }

  EventListener::~EventListener()
  {
    Remove();
  }

  EventListener::EventListener( EventListener && other ) noexcept
    : m_Window( other.m_Window )
    , m_Id( other.m_Id )
  {
    other.m_Window = nullptr;
    other.m_Id     = 0;
  }

  EventListener & EventListener::operator=( EventListener && other ) noexcept
  {
    if ( this != &other )
    {
      Remove();

      m_Window       = other.m_Window;
      m_Id           = other.m_Id;
      other.m_Window = nullptr;
      other.m_Id     = 0;
    }

    return *this;
  }

  bool EventListener::IsValid() const
  {
    return m_Window != nullptr && m_Id != 0;
  }

  void EventListener::Remove()
  {
    if ( m_Window && m_Id != 0 )
    {
      m_Window->RemoveEventListener( m_Id );
      m_Window = nullptr;
      m_Id     = 0;
    }
  }
} // namespace Engine::Platform::Events
