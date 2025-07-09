#include <chrono>
#include <iostream>

#include "Engine/Utility/Logger.hpp"

namespace Engine::Utility
{
  Logger::LogSeverity Logger::s_LogSeverity         = LogSeverity::m_Info;
  bool                Logger::s_IsDebugBreakEnabled = false;

  void Logger::SetSeverity( const LogSeverity severity )
  {
    s_LogSeverity = severity;
  }

  void Logger::Log( const LogMetadata & metadata, const LogMessage & message )
  {
    LogImpl( metadata, message );
  }

  void Logger::LogImpl( const LogMetadata & metadata,
                        const LogMessage &  message )
  {
    auto pLevelStr  = "";
    auto pColorCode = "";

    switch ( metadata.m_Severity )
    {
      case LogSeverity::m_Trace:
      {
        pLevelStr  = "TRACE";
        pColorCode = "\033[37m";
        break;
      }

      case LogSeverity::m_Info:
      {
        pLevelStr  = "INFO";
        pColorCode = "\033[32m";
        break;
      }

      case LogSeverity::m_Warn:
      {
        pLevelStr  = "WARN";
        pColorCode = "\033[33m";
        break;
      }

      case LogSeverity::m_Error:
      {
        pLevelStr  = "ERROR";
        pColorCode = "\033[31m";
        break;
      }

      case LogSeverity::m_Fatal:
      {
        pLevelStr             = "FATAL";
        pColorCode            = "\033[35m";
        s_IsDebugBreakEnabled = true;
        break;
      }
    }

    const auto Now   = std::chrono::system_clock::now();
    const auto TimeT = std::chrono::system_clock::to_time_t( Now );
    const auto Ms    = std::chrono::duration_cast<std::chrono::milliseconds>(
                      Now.time_since_epoch() ) %
                    1000;

    std::stringstream ss;
    ss << std::put_time( std::localtime( &TimeT ), "%H:%M:%S" );
    ss << '.' << std::setfill( '0' ) << std::setw( 3 ) << Ms.count();

    // [HH:MM:SS.mm] [LEVEL] file:line(function) message
    std::cout << pColorCode << '[' << ss.str() << "] [" << pLevelStr << "] "
              << metadata.m_SourceInfo.m_pFileName << ':'
              << metadata.m_SourceInfo.m_Line << "("
              << metadata.m_SourceInfo.m_pFunctionName << ") "
              << message.m_FormattedMessage << "\033[0m" << std::endl;

    if ( s_IsDebugBreakEnabled )
    {
      __debugbreak();
    }
  }
} // namespace Engine::Utility
