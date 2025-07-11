#include <Engine/Utility/Logger.hpp>

#include "Main.hpp"

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
