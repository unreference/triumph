#pragma once

#if defined( PLATFORM_WIN32 )

#include <Windows.h>

#include "Engine/Platform/Window.hpp"
#include "Engine/Core/Macro.hpp"

namespace Engine::Platform::Win32
{
  class Win32Window final : public Window
  {
    DISALLOW_COPY( Win32Window );
    DISALLOW_MOVE( Win32Window );

  public:
    explicit Win32Window( const WindowProps & props );
    ~Win32Window() override;

    [[nodiscard]] Result<vk::SurfaceKHR>
    CreateSurface( vk::Instance instance ) const override;

    void               PollEvents() override;
    void               SwapBuffers() override;
    [[nodiscard]] bool ShouldClose() const override;
    void               Close() override;

    [[nodiscard]] std::vector<const char *>
                              GetRequiredExtensions() const override;
    [[nodiscard]] std::string GetTitle() const override;
    [[nodiscard]] u32         GetWidth() const override;
    [[nodiscard]] u32         GetHeight() const override;
    [[nodiscard]] bool        IsVsynced() const override;
    [[nodiscard]] bool        IsFullScreen() const override;

    [[nodiscard]] void * GetNativeHandle() const override;

    void SetTitle( const std::string & title ) override;
    void SetSize( u32 width, u32 height ) override;
    void SetVsync( bool isEnabled ) override;
    void SetFullScreen( bool isEnabled ) override;

    u64  AddEventListener( EventCallback callback ) override;
    bool RemoveEventListener( u64 id ) override;
    void ClearEventListeners() override;

  private:
    struct WindowData
    {
      std::string m_Title;
      u32         m_Width;
      u32         m_Height;

      bool m_IsVsynced;
      bool m_IsFullScreen;
      bool m_ShouldClose;

      RECT  m_WindowedRect;
      DWORD m_WindowedStyle;
    };

    void Init( const WindowProps & props );

    static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParm );
    LRESULT HandleMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    HWND       m_pHandle;
    HINSTANCE  m_pInstance;
    WindowData m_Data;

    static bool           s_IsClassRegistered;
    static constexpr auto s_pClassName = L"Win32Window";
  };
} // namespace Engine::Platform::Win32

#endif
