#include "Engine/Utility/Logger.hpp"

#include "Engine/Platform/Window.hpp"

namespace Engine::Platform
{
  Window::Window()
    : m_NextListenerId( 1 )
  {
  }

  u64 Window::AddEventListener( EventCallback callback )
  {
    if ( !callback )
    {
      LOG_WARN( "Attempted to add null event callback!" );
      return 0;
    }

    u64 id = m_NextListenerId++;
    m_EventListeners.emplace_back(
      EventListener { id, std::move( callback ) } );

    LOG_INFO( "Added event listener with ID: {}", id );
    return id;
  }

  bool Window::RemoveEventListener( u64 id )
  {
    auto i = std::find_if( m_EventListeners.begin(), m_EventListeners.end(),
                           [ id ]( const EventListener & listener )
                           { return listener.id == id; } );

    if ( i != m_EventListeners.end() )
    {
      LOG_INFO( "Removed event listener with ID: {}", id );
      m_EventListeners.erase( i );
      return true;
    }

    LOG_WARN( "Attempted to remove non-existent event listener with ID: {}",
              id );
    return false;
  }

  void Window::ClearEventListeners()
  {
    LOG_INFO( "Clearing {} event listeners", m_EventListeners.size() );
    m_EventListeners.clear();
  }

  void Window::DispatchEvent( const Events::WindowEvent & event )
  {
    for ( const auto & listener : m_EventListeners )
    {
      try
      {
        listener.callback( event );
      }
      catch ( const std::exception & E )
      {
        LOG_ERROR( "Exception in event listener {}: {}", listener.id,
                   E.what() );
      }
      catch ( ... )
      {
        LOG_ERROR( "Unknown exception in event listener {}", listener.id );
      }
    }
  }

} // namespace Engine::Platform
