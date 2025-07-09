#pragma once

#include <memory>

#include "Macro.hpp"
#include "Types.hpp"
#include "Engine/Platform/IWindow.hpp"

namespace Engine::Renderer
{
  class Renderer;
}

namespace Engine::Core
{
  class ApplicationBase
  {
    DISALLOW_COPY( ApplicationBase );
    DISALLOW_MOVE( ApplicationBase );

  public:
    ApplicationBase();
    virtual ~ApplicationBase();

    void Run();
    void Close();

    [[nodiscard]] Platform::IWindow &  GetWindow() const;
    [[nodiscard]] Renderer::Renderer & GetRenderer() const;

  protected:
    virtual void Init()                                         = 0;
    virtual void Update( f32 deltaTime )                        = 0;
    virtual void Draw()                                         = 0;
    virtual void Shutdown()                                     = 0;
    virtual void OnEvent( const Platform::WindowEvent & event ) = 0;

  private:
    void InternalInit();
    void InternalShutdown();
    void HandleEvent( const Platform::WindowEvent & event );

    std::unique_ptr<Platform::IWindow>  m_Window;
    std::unique_ptr<Renderer::Renderer> m_Renderer;

    bool m_IsRunning;
    f32  m_LastFrameTime;
  };

  ApplicationBase * Create();

} // namespace Engine::Core
