#pragma once

#include <Engine/Core/ApplicationBase.hpp>

namespace Game
{
  class Application final : public Engine::Core::ApplicationBase
  {
  public:
    Application();
    ~Application() = default;

  protected:
    void Init() override;
    void Update( f32 deltaTime ) override;
    void Draw() override;
    void Shutdown() override;

  private:
    void SetupGameEventListeners();

    Engine::Platform::Events::KeyPressedListener m_EscapeKeyPressedListener;
    Engine::Platform::Events::KeyPressedListener m_KeyPressedListener;
    Engine::Platform::Events::MouseButtonPressedListener
      m_MouseButtonPressedListener;

    f32 m_TotalTime;
  };
} // namespace Game
