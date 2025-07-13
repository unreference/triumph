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

#include <Engine/Core/ApplicationBase.hpp>

namespace Game
{
  class Application final : public Engine::Core::ApplicationBase
  {
  public:
    Application();
    ~Application() override = default;

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
