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

#include <string_view>
#include <string>
#include <format>

#include "Engine/Core/Types.hpp"

namespace Engine::Utility::String
{
  std::string ToUtf8( std::u16string_view utf16Str );
  std::string ToUtf8( const std::u16string & utf16Str );
  std::string ToUtf8( const c16 * utf16Str );
  std::string ToUtf8( std::u32string_view utf32Str );
  std::string ToUtf8( const std::u32string & utf32Str );
  std::string ToUtf8( const c32 * utf32Str );

  std::u16string ToUtf16( std::string_view str );
  std::u16string ToUtf16( const std::string & str );
  std::u16string ToUtf16( const c8 * str );
  std::u16string ToUtf16( std::u32string_view utf32Str );

  std::u32string ToUtf32( std::string_view str );
  std::u32string ToUtf32( const std::string & str );
  std::u32string ToUtf32( const c8 * str );
  std::u32string ToUtf32( std::u16string_view utf16Str );

  bool IsUtf8( std::string_view str );
  bool IsUtf16( std::u16string_view utf16Str );
  bool IsUtf32( std::u32string_view utf32Str );

  template <typename... Args>
  std::string Format( std::format_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }

  template <typename... Args>
  std::u16string Format( std::format_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }

  template <typename... Args>
  std::u32string Format( std::format_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }

#if defined( _WIN32 )
  std::string ToUtf8( std::wstring_view wideStr );
  std::string ToUtf8( const std::wstring & wideStr );
  std::string ToUtf8( const wchar_t * wideStr );

  std::wstring ToWide( std::string_view str );
  std::wstring ToWide( const std::string & str );
  std::wstring ToWide( const c8 * str );

  void ToUtf8( std::wstring_view wideStr, std::string & str );
  void ToUtf8( std::u16string_view utf16Str, std::string & str );
  void ToUtf8( std::u32string_view utf32Str, std::string & str );
  void ToUtf16( std::string_view str, std::u16string & utf16Str );
  void ToUtf32( std::string_view str, std::u32string & utf32Str );
  void ToWide( std::string_view str, std::wstring & wideStr );

  bool IsWide( std::wstring_view wideStr );

  std::string GetWin32Error( u32 code );
  std::string GetLastWin32Error();

  template <typename... Args>
  std::wstring Format( std::wformat_string<Args...> fmt, Args &&... args )
  {
    return std::format( fmt, std::forward<Args>( args )... );
  }
#endif

  std::string Trim( std::string_view str );
  std::string TrimLeft( std::string_view str );
  std::string TrimRight( std::string_view str );
} // namespace Engine::Utility::String
