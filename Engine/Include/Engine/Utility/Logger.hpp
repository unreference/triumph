#pragma once

#include <format>
#include <source_location>

#include "Engine/Core/Types.hpp"

namespace Engine::Utility
{
  class Logger
  {
  public:
    enum class LogSeverity : u8
    {
      m_Trace,
      m_Info,
      m_Warn,
      m_Error,
      m_Fatal,
    };

    struct SourceInfo
    {
      u16          m_Line          = 0;
      const char * m_pFileName     = nullptr;
      const char * m_pFunctionName = nullptr;
    };

    struct LogMetadata
    {
      SourceInfo  m_SourceInfo = {};
      LogSeverity m_Severity   = {};
      bool        m_IsVerbose  = false;
    };

    struct LogMessage
    {
      std::string_view m_Format           = {};
      std::string      m_FormattedMessage = {};
    };

    static void SetSeverity( LogSeverity severity );
    static void Log( const LogMetadata & metadata, const LogMessage & message );

    template <typename... Args>
    static void Log( const std::source_location & loc,
                     const LogSeverity severity, const std::string_view fmt,
                     Args &&... args )
    {
      if ( severity < s_LogSeverity )
      {
        return;
      }

      LogMetadata meta                  = {};
      meta.m_SourceInfo.m_Line          = static_cast<u16>( loc.line() );
      meta.m_SourceInfo.m_pFileName     = loc.file_name();
      meta.m_SourceInfo.m_pFunctionName = loc.function_name();
      meta.m_Severity                   = severity;
      meta.m_IsVerbose                  = true;

      LogMessage message = {};
      message.m_Format   = fmt;

      if constexpr ( sizeof...( args ) > 0 )
      {
        message.m_FormattedMessage = std::vformat(
          fmt, std::make_format_args( std::forward<Args>( args )... ) );
      }
      else
      {
        message.m_FormattedMessage = std::string( fmt );
      }

      Log( meta, message );
    }

    template <typename... Args>
    static void Trace( const std::source_location & loc, std::string_view fmt,
                       Args &&... args )
    {
      Log( loc, LogSeverity::m_Trace, fmt, std::forward<Args>( args )... );
    }
    template <typename... Args>
    static void Info( const std::source_location & loc, std::string_view fmt,
                      Args &&... args )
    {
      Log( loc, LogSeverity::m_Info, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void Warn( const std::source_location & loc, std::string_view fmt,
                      Args &&... args )
    {
      Log( loc, LogSeverity::m_Warn, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void Error( const std::source_location & loc, std::string_view fmt,
                       Args &&... args )
    {
      Log( loc, LogSeverity::m_Error, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void Fatal( const std::source_location & loc, std::string_view fmt,
                       Args &&... args )
    {
      Log( loc, LogSeverity::m_Fatal, fmt, std::forward<Args>( args )... );
    }

  private:
    static void LogImpl( const LogMetadata & metadata,
                         const LogMessage &  message );

    static LogSeverity s_LogSeverity;
    static bool        s_IsDebugBreakEnabled;
  };
} // namespace Engine::Utility

#define LOG_TRACE( fmt, ... )                                                  \
  Engine::Utility::Logger::Trace( std::source_location::current(), fmt,        \
                                  ##__VA_ARGS__ )

#define LOG_INFO( fmt, ... )                                                   \
  Engine::Utility::Logger::Info( std::source_location::current(), fmt,         \
                                 ##__VA_ARGS__ )

#define LOG_WARN( fmt, ... )                                                   \
  Engine::Utility::Logger::Warn( std::source_location::current(), fmt,         \
                                 ##__VA_ARGS__ )

#define LOG_ERROR( fmt, ... )                                                  \
  Engine::Utility::Logger::Error( std::source_location::current(), fmt,        \
                                  ##__VA_ARGS__ )

#define LOG_FATAL( fmt, ... )                                                  \
  Engine::Utility::Logger::Fatal( std::source_location::current(), fmt,        \
                                  ##__VA_ARGS__ )
