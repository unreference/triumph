/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include <functional>

#include "Engine/Utility/Logger.hpp"

#include "Engine/Platform/Window.hpp"

namespace Engine::Platform
{
  Window::Window()
    : m_NextListenerId( 1 )
  {
  }

  u8 Window::AddEventListener( EventCallback callback )
  {
    if ( !callback )
    {
      LOG_WARN( "Attempted to add null event callback!" );
      return 0;
    }

    u8 id = m_NextListenerId++;
    m_EventListeners.emplace_back( EventListener { id, std::move( callback ) } );

    LOG_INFO( "Added event listener with ID: {}", id );
    return id;
  }

  bool Window::RemoveEventListener( u8 id )
  {
    const auto I = std::ranges::find_if( m_EventListeners,
                                         [ id ]( const EventListener & listener )
                                         { return listener.m_Id == id; } );

    if ( I != m_EventListeners.end() )
    {
      LOG_INFO( "Removed event listener with ID: {}", id );
      m_EventListeners.erase( I );
      return true;
    }

    LOG_WARN( "Attempted to remove non-existent event listener with ID: {}", id );
    return false;
  }

  void Window::ClearEventListeners()
  {
    LOG_INFO( "Clearing {} event listeners", m_EventListeners.size() );
    m_EventListeners.clear();
  }

  void Window::DispatchEvent( const Events::WindowEvent & event ) const
  {
    for ( const auto & [ Id, callback ] : m_EventListeners )
    {
      try
      {
        callback( event );
      }
      catch ( const std::exception & E )
      {
        LOG_ERROR( "Exception in event listener {}: {}", Id, E.what() );
      }
      catch ( ... )
      {
        LOG_ERROR( "Unknown exception in event listener {}", Id );
      }
    }
  }

} // namespace Engine::Platform
