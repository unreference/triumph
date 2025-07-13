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
    static void Log( const std::source_location & loc, const LogSeverity severity,
                     std::string_view fmt, Args &&... args )
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
        try
        {
          const std::string FmtStr( fmt );
          message.m_FormattedMessage =
            std::vformat( FmtStr, std::make_format_args( args... ) );
        }
        catch ( const std::format_error & e )
        {
          message.m_FormattedMessage =
            std::format( "[FORMAT ERROR: {}] Raw format: {}", e.what(), fmt );
        }
        catch ( ... )
        {
          message.m_FormattedMessage =
            std::format( "[UNKNOWN FORMAT ERROR] Raw format: {}", fmt );
        }
      }
      else
      {
        message.m_FormattedMessage = std::string( fmt );
      }

      Log( meta, message );
    }

    template <typename... Args>
    static void TraceImpl( const std::source_location & loc, std::string_view fmt,
                           Args &&... args )
    {
      Log( loc, LogSeverity::m_Trace, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void InfoImpl( const std::source_location & loc, std::string_view fmt,
                          Args &&... args )
    {
      Log( loc, LogSeverity::m_Info, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void WarnImpl( const std::source_location & loc, std::string_view fmt,
                          Args &&... args )
    {
      Log( loc, LogSeverity::m_Warn, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void ErrorImpl( const std::source_location & loc, std::string_view fmt,
                           Args &&... args )
    {
      Log( loc, LogSeverity::m_Error, fmt, std::forward<Args>( args )... );
    }

    template <typename... Args>
    static void FatalImpl( const std::source_location & loc, std::string_view fmt,
                           Args &&... args )
    {
      Log( loc, LogSeverity::m_Fatal, fmt, std::forward<Args>( args )... );

      std::abort();
    }

  private:
    static void LogImpl( const LogMetadata & metadata, const LogMessage & message );

    static LogSeverity s_LogSeverity;
    static bool        s_IsDebugBreakEnabled;
  };
} // namespace Engine::Utility

#define LOG_TRACE( fmt, ... )                                                       \
  Engine::Utility::Logger::TraceImpl( std::source_location::current(), fmt,         \
                                      ##__VA_ARGS__ )

#define LOG_INFO( fmt, ... )                                                        \
  Engine::Utility::Logger::InfoImpl( std::source_location::current(), fmt,          \
                                     ##__VA_ARGS__ )

#define LOG_WARN( fmt, ... )                                                        \
  Engine::Utility::Logger::WarnImpl( std::source_location::current(), fmt,          \
                                     ##__VA_ARGS__ )

#define LOG_ERROR( fmt, ... )                                                       \
  Engine::Utility::Logger::ErrorImpl( std::source_location::current(), fmt,         \
                                      ##__VA_ARGS__ )

#define LOG_FATAL( fmt, ... )                                                       \
  Engine::Utility::Logger::FatalImpl( std::source_location::current(), fmt,         \
                                      ##__VA_ARGS__ )
