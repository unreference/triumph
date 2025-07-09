#pragma once

#include <memory>

#include "Window.hpp"

#if defined( PLATFORM_WIN32 )
#include "Win32/Win32Window.hpp"
#elif defined( PLATFORM_UNIX )
#include "Unix/UnixWindow.hpp"
#endif

namespace Engine::Platform
{
  inline std::unique_ptr<Window> Window::Create( const WindowProps & props )
  {
#if defined( PLATFORM_WIN32 )
    return std::make_unique<Win32::Win32Window>( props );
#elif defined( PLATFORM_UNIX )
    return std::make_unique<UnixWindow>( props );
#else
    static_assert( false, "Unsupported platform" );
#endif
  }
} // namespace Engine::Platform
