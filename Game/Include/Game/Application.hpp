#pragma once

#include <Engine/Core/ApplicationBase.hpp>
#include <Engine/Core/Macro.hpp>

namespace Game
{
  class Application final : public Engine::Core::ApplicationBase
  {
    DISALLOW_COPY( Application );
    DISALLOW_MOVE( Application );

  public:
    Application();
    ~Application() override = default;

  protected:
    void Init() override;
    void Update( f32 deltaTime ) override;
    void Draw() override;
    void Shutdown() override;
    void OnEvent( const Engine::Platform::WindowEvent & event ) override;

  private:
    f32 m_TotalTime;
  };
} // namespace Game
