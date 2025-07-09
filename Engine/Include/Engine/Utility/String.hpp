#pragma once

#include <string_view>
#include <string>
#include <format>

#include "Engine/Core/Types.hpp"

namespace Engine::Utility::String
{
  std::string ToUtf8( std::wstring_view utf16 );
  std::string ToUtf8( const std::wstring & utf16 );
  std::string ToUtf8( const wchar_t * utf16 );

  std::wstring ToUtf16( std::string_view utf8 );
  std::wstring ToUtf16( const std::string & utf8 );
  std::wstring ToUtf16( const char * utf8 );

  void ToUtf8( std::wstring_view utf16, std::string & utf8 );
  void ToUtf16( std::string_view utf8, std::wstring & utf16 );

#if defined( PLATFORM_WIN32 )
  std::string GetWin32Error( u32 errorCode );
  std::string GetLastWin32Error();
#endif

  bool IsUtf8( std::string_view string );
  bool IsUtf16( std::wstring_view string );

  template <typename... Args>
  std::string Format( std::format_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }

  template <typename... Args>
  std::wstring Format( std::wformat_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }

  std::string Trim( std::string_view string );
  std::string TrimLeft( std::string_view string );
  std::string TrimRight( std::string_view string );
} // namespace Engine::Utility::String
