#if defined( PLATFORM_WIN32 )

#include <vector>
#include <memory>
#include <exception>

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Result.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Platform/Win32/Win32Window.hpp"

namespace Engine::Platform::Win32
{
  bool Win32Window::s_IsClassRegistered = false;

  Win32Window::Win32Window( const WindowProps & props )
    : m_pHandle( nullptr )
    , m_pInstance( nullptr )
  {
    Init( props );
  }

  Win32Window::~Win32Window()
  {
    if ( m_pHandle )
    {
      DestroyWindow( m_pHandle );
      m_pHandle = nullptr;
    }
  }

  Result<vk::SurfaceKHR>
  Win32Window::CreateSurface( const vk::Instance instance ) const
  {
    vk::Win32SurfaceCreateInfoKHR createInfo = {};
    createInfo.hinstance                     = m_pInstance;
    createInfo.hwnd                          = m_pHandle;

    try
    {
      vk::SurfaceKHR surface = instance.createWin32SurfaceKHR( createInfo );
      return surface;
    }
    catch ( const vk::SystemError & e )
    {
      return std::format( "Failed to create Vulkan surface: {}", e.what() );
    }
  }

  void Win32Window::PollEvents()
  {
    MSG msg = {};
    while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
    {
      TranslateMessage( &msg );
      DispatchMessageW( &msg );
    }
  }

  void Win32Window::SwapBuffers()
  {
    // #STUB: Not needed for Vulkan
  }

  bool Win32Window::ShouldClose() const
  {
    return m_Data.m_ShouldClose;
  }

  void Win32Window::Close()
  {
    m_Data.m_ShouldClose = true;
  }

  std::vector<const char *> Win32Window::GetRequiredExtensions() const
  {
    return { vk::KHRSurfaceExtensionName, vk::KHRWin32SurfaceExtensionName };
  }

  std::string Win32Window::GetTitle() const
  {
    return m_Data.m_Title;
  }

  u32 Win32Window::GetWidth() const
  {
    return m_Data.m_Width;
  }

  u32 Win32Window::GetHeight() const
  {
    return m_Data.m_Height;
  }

  bool Win32Window::IsVsynced() const
  {
    return m_Data.m_IsVsynced;
  }

  bool Win32Window::IsFullScreen() const
  {
    return m_Data.m_IsFullScreen;
  }

  void * Win32Window::GetNativeHandle() const
  {
    return m_pHandle;
  }

  void Win32Window::SetTitle( const std::string & title )
  {
    m_Data.m_Title = title;

    const int Length =
      MultiByteToWideChar( CP_UTF8, 0, title.c_str(), -1, nullptr, 0 );
    std::wstring wide( Length, 0 );
    MultiByteToWideChar( CP_UTF8, 0, title.c_str(), -1, wide.data(), Length );

    SetWindowTextW( m_pHandle, wide.c_str() );
  }

  void Win32Window::SetSize( const u32 width, const u32 height )
  {
    m_Data.m_Width = width;
    m_Data.m_Width = height;

    RECT        rect  = { 0, 0, static_cast<LONG>( width ),
                          static_cast<LONG>( height ) };
    const DWORD Style = GetWindowLongPtrW( m_pHandle, GWL_STYLE );
    AdjustWindowRect( &rect, Style, FALSE );

    SetWindowPos( m_pHandle, nullptr, 0, 0, rect.right - rect.left,
                  rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER );
  }

  void Win32Window::SetVsync( const bool isEnabled )
  {
    m_Data.m_IsVsynced = isEnabled;
  }

  void Win32Window::SetFullScreen( const bool isEnabled )
  {
    if ( m_Data.m_IsFullScreen == isEnabled )
    {
      return;
    }

    m_Data.m_IsFullScreen = isEnabled;

    if ( isEnabled )
    {
      GetWindowRect( m_pHandle, &m_Data.m_WindowedRect );
      m_Data.m_WindowedStyle =
        static_cast<DWORD>( GetWindowLongPtrW( m_pHandle, GWL_STYLE ) );

      HMONITOR Monitor =
        MonitorFromWindow( m_pHandle, MONITOR_DEFAULTTONEAREST );
      MONITORINFO info = { sizeof( MONITORINFO ) };
      GetMonitorInfoW( Monitor, &info );

      SetWindowLongPtrW( m_pHandle, GWL_STYLE, WS_POPUP );
      SetWindowPos(
        m_pHandle, HWND_TOP, info.rcMonitor.left, info.rcMonitor.top,
        info.rcMonitor.right - info.rcMonitor.left,
        info.rcMonitor.bottom - info.rcMonitor.top, SWP_FRAMECHANGED );
    }
    else
    {
      SetWindowLongPtrW( m_pHandle, GWL_STYLE, m_Data.m_WindowedStyle );
      SetWindowPos( m_pHandle, nullptr, m_Data.m_WindowedRect.left,
                    m_Data.m_WindowedRect.top,
                    m_Data.m_WindowedRect.right - m_Data.m_WindowedRect.left,
                    m_Data.m_WindowedRect.bottom - m_Data.m_WindowedRect.top,
                    SWP_FRAMECHANGED );
    }
  }

  void Win32Window::SetEventCallback( EventCallback callback )
  {
    m_Data.m_EventCallback = std::move( callback );
  }

  void Win32Window::Init( const WindowProps & props )
  {
    m_Data.m_Title        = props.m_Title;
    m_Data.m_Width        = props.m_Width;
    m_Data.m_Height       = props.m_Height;
    m_Data.m_IsVsynced    = props.m_IsVsynced;
    m_Data.m_IsFullScreen = props.m_IsFullScreen;
    m_Data.m_ShouldClose  = false;

    m_pInstance = GetModuleHandleW( nullptr );

    if ( !s_IsClassRegistered )
    {
      WNDCLASSEXW wc = {};
      wc.cbSize      = sizeof( WNDCLASSEXW );
      wc.style       = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = WindowProc;
      wc.hInstance   = m_pInstance;
      wc.hIcon =
        LoadIconW( nullptr, reinterpret_cast<LPCWSTR>( IDI_APPLICATION ) );
      wc.hCursor =
        LoadCursorW( nullptr, reinterpret_cast<LPCWSTR>( IDC_ARROW ) );
      wc.hbrBackground = nullptr;
      wc.lpszClassName = s_pClassName;
      wc.hIconSm =
        LoadIconW( nullptr, reinterpret_cast<LPCWSTR>( IDI_APPLICATION ) );

      if ( !RegisterClassExW( &wc ) )
      {
        throw std::runtime_error( "Failed to register Win32 window class!" );
      }

      s_IsClassRegistered = true;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    if ( !props.m_IsResizable )
    {
      style &= WS_THICKFRAME | WS_MAXIMIZEBOX;
    }

    if ( !props.m_IsDecorated )
    {
      style = WS_POPUP;
    }

    RECT windowRect = { 0, 0, static_cast<LONG>( props.m_Width ),
                        static_cast<LONG>( props.m_Height ) };
    AdjustWindowRect( &windowRect, style, FALSE );

    const i32 WindowWidth  = windowRect.right - windowRect.left;
    const i32 WindowHeight = windowRect.bottom - windowRect.top;

    const i32 ScreenWidth  = GetSystemMetrics( SM_CXSCREEN );
    const i32 ScreenHeight = GetSystemMetrics( SM_CYSCREEN );
    const i32 WindowX      = ( ScreenWidth - WindowWidth ) / 2;
    const i32 WindowY      = ( ScreenHeight - WindowHeight ) / 2;

    const int Length =
      MultiByteToWideChar( CP_UTF8, 0, props.m_Title.c_str(), -1, nullptr, 0 );
    std::wstring wideTitle( Length, 0 );
    MultiByteToWideChar( CP_UTF8, 0, props.m_Title.c_str(), -1,
                         wideTitle.data(), Length );

    m_pHandle = CreateWindowExW( 0, s_pClassName, wideTitle.c_str(), style,
                                 WindowX, WindowY, WindowWidth, WindowHeight,
                                 nullptr, nullptr, m_pInstance, &m_Data );

    if ( !m_pHandle )
    {
      throw std::runtime_error( "Failed to create Win32 window!" );
    }

    GetWindowRect( m_pHandle, &m_Data.m_WindowedRect );
    m_Data.m_WindowedStyle = style;

    ShowWindow( m_pHandle, SW_SHOW );
    UpdateWindow( m_pHandle );

    if ( props.m_IsFullScreen )
    {
      SetFullScreen( m_pHandle );
    }

    LOG_INFO( "Successfully created Win32 window (title={}, width ={}, "
              "height={}, vsynced={})",
              m_Data.m_Title, m_Data.m_Width, m_Data.m_Height,
              m_Data.m_IsVsynced );
  }

  LRESULT CALLBACK Win32Window::WindowProc( HWND hWnd, const UINT uMsg,
                                            const WPARAM wParam,
                                            const LPARAM lParm )
  {
    WindowData * pData = nullptr;

    if ( uMsg == WM_NCCREATE )
    {
      const auto pCreateStruct = reinterpret_cast<CREATESTRUCTW *>( lParm );

      pData = static_cast<WindowData *>( pCreateStruct->lpCreateParams );
      SetWindowLongPtrW( hWnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>( pData ) );
    }
    else
    {
      pData = reinterpret_cast<WindowData *>(
        GetWindowLongPtrW( hWnd, GWLP_USERDATA ) );
    }

    if ( !pData )
    {
      return DefWindowProcW( hWnd, uMsg, wParam, lParm );
    }

    const auto pWindow = reinterpret_cast<Win32Window *>(
      reinterpret_cast<char *>( pData ) - offsetof( Win32Window, m_Data ) );
    return pWindow->HandleMessage( uMsg, wParam, lParm );
  }

  LRESULT Win32Window::HandleMessage( const UINT uMsg, WPARAM wParam,
                                      const LPARAM lParam )
  {
    if ( !m_Data.m_EventCallback )
    {
      return DefWindowProcW( m_pHandle, uMsg, wParam, lParam );
    }

    switch ( uMsg )
    {
      case WM_CLOSE:
      {
        WindowCloseEvent event = {};
        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_SIZE:
      {
        const u32 NewWidth  = LOWORD( lParam );
        const u32 NewHeight = HIWORD( lParam );

        m_Data.m_Width  = NewWidth;
        m_Data.m_Height = NewHeight;

        WindowResizeEvent event = {};
        event.m_Width           = NewHeight;
        event.m_Height          = NewHeight;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_SETFOCUS:
      {
        WindowSetFocusEvent event = {};
        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_KILLFOCUS:
      {
        WindowKillFocusEvent event = {};
        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MOVE:
      {
        const i32 X = LOWORD( lParam );
        const i32 Y = HIWORD( lParam );

        WindowMovedEvent event = {};
        event.m_X              = X;
        event.m_Y              = Y;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_KEYDOWN:
        /* fall through */
      case WM_SYSKEYDOWN:
      {
        const auto Key = static_cast<KeyCode>( wParam );
        const u8   RepeatCount =
          static_cast<u8>( lParam & 0xFF ); // lParam & 0xFFFF
                                            // for UNICODE?

        KeyPressedEvent event = {};
        event.m_Key           = Key;
        event.m_RepeatCount   = RepeatCount;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_KEYUP:
        /* fall through */
      case WM_SYSKEYUP:
      {
        const auto Key = static_cast<KeyCode>( wParam );

        KeyReleasedEvent event = {};
        event.m_Key            = Key;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_CHAR:
      {
        const char Character = static_cast<char>( wParam ); // u32 for UNICODE?

        KeyTypedEvent event = {};
        event.m_Character   = Character;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_LBUTTONDOWN:
      {
        MouseButtonPressedEvent event = {};
        event.m_Button                = MouseButton::m_Left;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_LBUTTONUP:
      {
        MouseButtonReleasedEvent event = {};
        event.m_Button                 = MouseButton::m_Left;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_RBUTTONDOWN:
      {
        MouseButtonPressedEvent event = {};
        event.m_Button                = MouseButton::m_Right;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_RBUTTONUP:
      {
        MouseButtonReleasedEvent event = {};
        event.m_Button                 = MouseButton::m_Right;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MBUTTONDOWN:
      {
        MouseButtonPressedEvent event = {};
        event.m_Button                = MouseButton::m_Middle;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MBUTTONUP:
      {
        MouseButtonReleasedEvent event = {};
        event.m_Button                 = MouseButton::m_Middle;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MOUSEMOVE:
      {
        const f32 X = LOWORD( lParam );
        const f32 Y = HIWORD( lParam );

        MouseMovedEvent event = {};
        event.m_X             = X;
        event.m_Y             = Y;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MOUSEWHEEL:
      {
        const f32 Delta = static_cast<f32>( GET_WHEEL_DELTA_WPARAM( wParam ) ) /
                          static_cast<f32>( WHEEL_DELTA );

        MouseScrolledEvent event = {};
        event.m_XOffset          = 0.0f;
        event.m_YOffset          = Delta;

        m_Data.m_EventCallback( event );
        return 0;
      }

      case WM_MOUSEHWHEEL:
      {
        const f32 Delta = static_cast<f32>( GET_WHEEL_DELTA_WPARAM( wParam ) ) /
                          static_cast<f32>( WHEEL_DELTA );

        MouseScrolledEvent event = {};
        event.m_XOffset          = Delta;
        event.m_YOffset          = 0.0f;

        m_Data.m_EventCallback( event );
        return 0;
      }

      default:
      {
        // Intentionally blank
      }
    }

    return DefWindowProcW( m_pHandle, uMsg, wParam, lParam );
  }
} // namespace Engine::Platform::Win32

#endif
