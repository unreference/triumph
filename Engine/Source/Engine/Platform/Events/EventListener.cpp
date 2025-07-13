#include "Engine/Utility/Logger.hpp"

#include "Engine/Platform/Events/EventListener.hpp"

namespace Engine::Platform::Events
{
  EventListener::EventListener( Window & window, Window::EventCallback callback )
    : m_pWindow( &window )
    , m_Id( 0 )
  {
    m_Id = m_pWindow->AddEventListener( std::move( callback ) );
    if ( m_Id == 0 )
    {
      LOG_FATAL( "Failed to add event listener!" );
      m_pWindow = nullptr;
    }
  }

  EventListener::~EventListener()
  {
    Remove();
  }

  EventListener::EventListener( EventListener && other ) noexcept
    : m_pWindow( other.m_pWindow )
    , m_Id( other.m_Id )
  {
    other.m_pWindow = nullptr;
    other.m_Id      = 0;
  }

  EventListener & EventListener::operator=( EventListener && other ) noexcept
  {
    if ( this != &other )
    {
      Remove();

      m_pWindow       = other.m_pWindow;
      m_Id            = other.m_Id;
      other.m_pWindow = nullptr;
      other.m_Id      = 0;
    }

    return *this;
  }

  bool EventListener::IsValid() const
  {
    return m_pWindow != nullptr && m_Id != 0;
  }

  void EventListener::Remove()
  {
    if ( m_pWindow && m_Id != 0 )
    {
      m_pWindow->RemoveEventListener( m_Id );
      m_pWindow = nullptr;
      m_Id      = 0;
    }
  }
} // namespace Engine::Platform::Events
