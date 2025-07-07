#include <Engine/Utility/Logger.hpp>

int main()
{
  LOG_TRACE( "Hello, Trace!" );
  LOG_INFO( "Hello, Info!" );
  LOG_WARN( "Hello, Warn!" );
  LOG_ERROR( "Hello, Error!" );
  LOG_FATAL( "Hello, Fatal!" );
}
