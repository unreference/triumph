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

#include <memory>

#include "Window.hpp"

#if defined( _WIN32 )
#include "Win32/Win32Window.hpp"
#elif defined( __linux__ )
#include "Linux/LinuxWindow.hpp"
#endif

namespace Engine::Platform
{
  inline std::unique_ptr<Window> Window::Create( const WindowProps & props )
  {
#if defined( _WIN32 )
    return std::make_unique<Win32::Win32Window>( props );
#elif defined( __linux__ )
    return std::make_unique<Linux::LinuxWindow>( props );
#else
    static_assert( false, "Unsupported platform" );
#endif
  }
} // namespace Engine::Platform
