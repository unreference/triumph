#include <Engine/Core/ApplicationBase.hpp>
#include <Engine/Utility/Logger.hpp>

#include "Game/Application.hpp"

Engine::Core::ApplicationBase * Engine::Core::Create()
{
  return new Game::Application();
}

int main()
{
  try
  {
    const auto pApp = Engine::Core::Create();
    pApp->Run();
    delete pApp;
    return 0;
  }
  catch ( const std::exception & E )
  {
    LOG_FATAL( E.what() );
  }
}
