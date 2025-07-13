/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#if defined( _WIN32 )
#include <Windows.h>
#endif

#include "Engine/Utility/String.hpp"

namespace Engine::Utility::String
{
  std::string ToUtf8( const std::u16string_view utf16Str )
  {
    if ( utf16Str.empty() )
    {
      return {};
    }

    std::string result;
    result.reserve( utf16Str.length() * 3 );

    for ( size i = 0; i < utf16Str.length(); /**/ )
    {
      const auto Unit1     = utf16Str[ i ];
      u32        codePoint = 0;

      if ( Unit1 < 0xD800 || Unit1 > 0xDFFF )
      {
        codePoint  = Unit1;
        i         += 1;
      }
      else if ( Unit1 >= 0xD800 && Unit1 <= 0xDBFF )
      {
        if ( i + 1 >= utf16Str.length() )
        {
          return {};
        }

        const auto Unit2 = utf16Str[ i + 1 ];
        if ( Unit2 < 0xDC00 || Unit2 > 0xDFFF )
        {
          return {};
        }

        codePoint  = 0x10000 + ( ( Unit1 & 0x3FF ) << 10 ) + ( Unit2 & 0x3FF );
        i         += 2;
      }
      else
      {
        return {};
      }

      if ( codePoint <= 0x7F )
      {
        result.push_back( static_cast<c8>( codePoint ) );
      }
      else if ( codePoint <= 0x7FF )
      {
        result.push_back( static_cast<c8>( 0xC0 | codePoint >> 6 ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint & 0x3F ) );
      }
      else if ( codePoint <= 0xFFFF )
      {
        result.push_back( static_cast<c8>( 0xE0 | codePoint >> 12 ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint >> 6 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint & 0x3F ) );
      }
      else
      {
        result.push_back( static_cast<c8>( 0xF0 | codePoint >> 18 ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint >> 12 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint >> 6 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | codePoint & 0x3F ) );
      }
    }

    return result;
  }

  std::string ToUtf8( const std::u16string & utf16Str )
  {
    return ToUtf8( std::u16string_view { utf16Str } );
  }

  std::string ToUtf8( const c16 * utf16Str )
  {
    return utf16Str ? ToUtf8( std::u16string_view { utf16Str } ) : std::string {};
  }

  std::string ToUtf8( const std::u32string_view utf32Str )
  {
    if ( utf32Str.empty() )
    {
      return {};
    }

    std::string result;
    result.reserve( utf32Str.length() * 4 );

    for ( const auto CodePoint : utf32Str )
    {
      if ( CodePoint > 0x10FFFF || ( CodePoint >= 0xD800 && CodePoint <= 0xDFFF ) )
      {
        return {};
      }

      if ( CodePoint <= 0x7F )
      {
        result.push_back( static_cast<c8>( CodePoint ) );
      }
      else if ( CodePoint <= 0x7FF )
      {
        result.push_back( static_cast<c8>( 0xC0 | CodePoint >> 6 ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint & 0x3F ) );
      }
      else if ( CodePoint <= 0xFFFF )

      {
        result.push_back( static_cast<c8>( 0xE0 | CodePoint >> 12 ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint >> 6 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint & 0x3F ) );
      }
      else
      {
        result.push_back( static_cast<c8>( 0xF0 | CodePoint >> 18 ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint >> 12 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint >> 6 & 0x3F ) );
        result.push_back( static_cast<c8>( 0x80 | CodePoint & 0x3F ) );
      }
    }

    return result;
  }

  std::string ToUtf8( const std::u32string & utf32Str )
  {
    return ToUtf8( std::u32string_view { utf32Str } );
  }

  std::string ToUtf8( const c32 * utf32Str )
  {
    return utf32Str ? ToUtf8( std::u32string_view { utf32Str } ) : std::string {};
  }

  std::u16string ToUtf16( const std::string_view str )
  {
    if ( str.empty() )
    {
      return {};
    }

    std::u16string result;
    result.reserve( str.length() );

    for ( size i = 0; i < str.length(); /**/ )
    {
      const auto Byte1     = static_cast<u8>( str[ i ] );
      u32        codePoint = 0;

      if ( Byte1 < 0x80 )
      {
        codePoint  = Byte1;
        i         += 1;
      }
      else if ( Byte1 >> 5 == 0b110 )
      {
        if ( i + 1 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0x1F ) << 6 | Byte2 & 0x3F;
        if ( codePoint < 0x80 )
        {
          return {};
        }

        i += 2;
      }
      else if ( Byte1 >> 4 == 0b1110 )
      {
        if ( i + 2 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0xF ) << 12 | ( Byte2 & 0x3F ) << 6 | Byte3 & 0x3F;
        if ( codePoint < 0x800 || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
        {
          return {};
        }

        i += 3;
      }
      else if ( Byte1 >> 3 == 0b11110 )
      {
        if ( i + 3 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        const auto Byte4 = static_cast<u8>( str[ i + 3 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 ||
             ( Byte4 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0x07 ) << 18 | ( Byte2 & 0x3F ) << 12 |
                    ( Byte3 & 0x3F ) << 6 | Byte4 & 0x3F;
        if ( codePoint < 0x10000 || codePoint > 0x10FFFF )
        {
          return {};
        }

        i += 4;
      }
      else
      {
        return {};
      }

      if ( codePoint <= 0xFFFF )
      {
        result.push_back( static_cast<c16>( codePoint ) );
      }
      else
      {
        codePoint -= 0x10000;
        result.push_back( static_cast<c16>( 0xD800 + ( codePoint >> 10 ) ) );
        result.push_back( static_cast<c16>( 0xDC00 + ( codePoint & 0x3FF ) ) );
      }
    }

    return result;
  }

  std::u16string ToUtf16( const std::string & str )
  {
    return ToUtf16( std::string_view { str } );
  }

  std::u16string ToUtf16( const c8 * str )
  {
    return str ? ToUtf16( std::string_view { str } ) : std::u16string {};
  }

  std::u16string ToUtf16( const std::u32string_view utf32Str )
  {
    if ( utf32Str.empty() )
    {
      return {};
    }

    std::u16string result;
    result.reserve( utf32Str.length() * 2 );

    for ( auto codePoint : utf32Str )
    {
      if ( codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
      {
        return {};
      }

      if ( codePoint <= 0xFFFF )
      {
        result.push_back( static_cast<c16>( codePoint ) );
      }
      else
      {
        codePoint -= 0x10000;
        result.push_back( static_cast<c16>( 0xD800 + ( codePoint >> 10 ) ) );
        result.push_back( static_cast<c16>( 0xDC00 + ( codePoint & 0x3FF ) ) );
      }
    }

    return result;
  }

  std::u32string ToUtf32( const std::string_view str )
  {
    if ( str.empty() )
    {
      return {};
    }

    std::u32string result;
    result.reserve( str.length() );

    for ( size i = 0; i < str.length(); /**/ )
    {
      const auto Byte1     = static_cast<u8>( str[ i ] );
      u32        codePoint = 0;

      if ( Byte1 < 0x80 )
      {
        codePoint  = Byte1;
        i         += 1;
      }
      else if ( Byte1 >> 5 == 0b110 )
      {
        if ( i + 1 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0x1F ) << 6 | Byte2 & 0x3F;
        if ( codePoint < 0x80 )
        {
          return {};
        }

        i += 2;
      }
      else if ( Byte1 >> 4 == 0b1110 )
      {
        if ( i + 2 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0x0F ) << 12 | ( Byte2 & 0x3F ) << 6 | Byte3 & 0x3F;
        if ( codePoint < 0x800 || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
        {
          return {};
        }

        i += 3;
      }
      else if ( Byte1 >> 3 == 0b11110 )
      {
        if ( i + 3 >= str.length() )
        {
          return {};
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        const auto Byte4 = static_cast<u8>( str[ i + 3 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 ||
             ( Byte4 & 0xC0 ) != 0x80 )
        {
          return {};
        }

        codePoint = ( Byte1 & 0x07 ) << 18 | ( Byte2 & 0x3F ) << 12 |
                    ( Byte3 & 0x3F ) << 6 | Byte4 & 0x3F;
        if ( codePoint < 0x10000 || codePoint > 0x10FFFF )
        {
          return {};
        }

        i += 4;
      }
      else
      {
        return {};
      }

      result.push_back( static_cast<c32>( codePoint ) );
    }

    return result;
  }

  std::u32string ToUtf32( const std::string & str )
  {
    return ToUtf32( std::string_view { str } );
  }

  std::u32string ToUtf32( const c8 * str )
  {
    return str ? ToUtf32( std::string_view { str } ) : std::u32string {};
  }

  std::u32string ToUtf32( const std::u16string_view utf16Str )
  {
    if ( utf16Str.empty() )
    {
      return {};
    }

    std::u32string result;
    result.reserve( utf16Str.length() );

    for ( size i = 0; i < utf16Str.length(); /**/ )
    {
      if ( const auto Unit1 = utf16Str[ i ]; Unit1 < 0xD800 || Unit1 > 0xDFFF )
      {
        result.push_back( Unit1 );
        i += 1;
      }
      else if ( Unit1 >= 0xD800 && Unit1 <= 0xDBFF )
      {
        if ( i + 1 >= utf16Str.length() )
        {
          return {};
        }

        const auto Unit2 = utf16Str[ i + 1 ];
        if ( Unit2 < 0xDC00 || Unit2 > 0xDFFF )
        {
          return {};
        }

        const auto CodePoint =
          0x10000 + ( ( Unit1 & 0x3FF ) << 10 ) + ( Unit2 & 0x3FF );
        result.push_back( static_cast<c32>( CodePoint ) );

        i += 2;
      }
      else
      {
        return {};
      }
    }

    return result;
  }

  bool IsUtf8( const std::string_view str )
  {
    for ( size i = 0; i < str.length(); /**/ )
    {
      if ( const auto Byte1 = static_cast<u8>( str[ i ] ); Byte1 < 0x80 )
      {
        i += 1;
      }
      else if ( Byte1 >> 5 == 0b110 )
      {
        if ( i + 1 >= str.length() )
        {
          return false;
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        if ( const auto CodePoint = ( Byte1 & 0x1F ) << 6 | Byte2 & 0x3F;
             CodePoint < 0x80 )
        {
          return false;
        }

        i += 2;
      }
      else if ( Byte1 >> 4 == 0b1110 )
      {
        if ( i + 2 >= str.length() )
        {
          return false;
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        const auto CodePoint =
          ( Byte1 & 0x0F ) << 12 | ( Byte2 & 0x3F ) << 6 | Byte3 & 0x3F;
        if ( CodePoint < 0x800 || ( CodePoint >= 0xD800 && CodePoint <= 0xDFFF ) )
        {
          return false;
        }

        i += 3;
      }
      else if ( Byte1 >> 3 == 0b11110 )
      {
        if ( i + 3 >= str.length() )
        {
          return false;
        }

        const auto Byte2 = static_cast<u8>( str[ i + 1 ] );
        const auto Byte3 = static_cast<u8>( str[ i + 2 ] );
        const auto Byte4 = static_cast<u8>( str[ i + 3 ] );
        if ( ( Byte2 & 0xC0 ) != 0x80 || ( Byte3 & 0xC0 ) != 0x80 ||
             ( Byte4 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        const auto CodePoint = ( Byte1 & 0x07 ) << 18 | ( Byte2 & 0x3F ) << 12 |
                               ( Byte3 & 0x3F ) << 6 | Byte4 & 0x3F;
        if ( CodePoint < 0x10000 || CodePoint > 0x10FFFF )
        {
          return false;
        }

        i += 4;
      }
      else
      {
        return false;
      }
    }

    return true;
  }

  bool IsUtf16( const std::u16string_view utf16Str )
  {
    for ( size i = 0; i < utf16Str.length(); /**/ )
    {
      if ( const auto Unit1 = utf16Str[ i ]; Unit1 < 0xD800 || Unit1 > 0xDFFF )
      {
        i += 1;
      }
      else if ( Unit1 >= 0xD800 && Unit1 <= 0xDBFF )
      {
        if ( i + 1 >= utf16Str.length() )
        {
          return false;
        }

        if ( const auto Unit2 = utf16Str[ i + 1 ]; Unit2 < 0xDC00 || Unit2 > 0xDFFF )
        {
          return false;
        }

        i += 2;
      }
      else
      {
        return false;
      }
    }

    return true;
  }

  bool IsUtf32( const std::u32string_view utf32Str )
  {
    for ( const auto CodePoint : utf32Str )
    {
      if ( CodePoint > 0x10FFFF || ( CodePoint >= 0xD800 && CodePoint <= 0xDFFF ) )
      {
        return false;
      }
    }

    return true;
  }

#if defined( _WIN32 )
  std::string ToUtf8( const std::wstring_view wideStr )
  {
    if ( wideStr.empty() )
    {
      return {};
    }

    const auto Length = WideCharToMultiByte( CP_UTF8, 0, wideStr.data(),
                                             static_cast<i32>( wideStr.length() ),
                                             nullptr, 0, nullptr, nullptr );
    if ( Length == 0 )
    {
      return {};
    }

    std::string result( Length, 0 );
    WideCharToMultiByte( CP_UTF8, 0, wideStr.data(),
                         static_cast<i32>( wideStr.length() ), result.data(), Length,
                         nullptr, nullptr );
    return result;
  }

  std::string ToUtf8( const std::wstring & wideStr )
  {
    return ToUtf8( std::wstring_view { wideStr } );
  }

  std::string ToUtf8( const wchar_t * wideStr )
  {
    return wideStr ? ToUtf8( std::wstring_view { wideStr } ) : std::string {};
  }

  std::wstring ToWide( const std::string_view str )
  {
    if ( str.empty() )
    {
      return {};
    }

    const auto Length = MultiByteToWideChar(
      CP_UTF8, 0, str.data(), static_cast<i32>( str.length() ), nullptr, 0 );
    if ( Length == 0 )
    {
      return {};
    }

    std::wstring result( Length, 0 );
    MultiByteToWideChar( CP_UTF8, 0, str.data(), static_cast<i32>( str.length() ),
                         result.data(), Length );
    return result;
  }

  std::wstring ToWide( const std::string & str )
  {
    return ToWide( std::string_view { str } );
  }

  std::wstring ToWide( const c8 * str )
  {
    return str ? ToWide( std::string_view { str } ) : std::wstring {};
  }

  void ToUtf8( const std::wstring_view wideStr, std::string & str )
  {
    str = ToUtf8( wideStr );
  }

  void ToUtf8( const std::u16string_view utf16Str, std::string & str )
  {
    str = ToUtf8( utf16Str );
  }

  void ToUtf8( const std::u32string_view utf32Str, std::string & str )
  {
    str = ToUtf8( utf32Str );
  }

  void ToUtf16( const std::string_view str, std::u16string & utf16Str )
  {
    utf16Str = ToUtf16( str );
  }

  void ToUtf32( const std::string_view str, std::u32string & utf32Str )
  {
    utf32Str = ToUtf32( str );
  }

  void ToWide( const std::string_view str, std::wstring & wideStr )
  {
    wideStr = ToWide( str );
  }

  bool IsWide( const std::wstring_view wideStr )
  {
    const std::u16string_view Utf16( reinterpret_cast<const c16 *>( wideStr.data() ),
                                     wideStr.length() );
    return IsUtf16( Utf16 );
  }

  std::string GetWin32Error( const u32 code )
  {
    LPSTR      pBuffer = nullptr;
    const auto Length =
      FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                      reinterpret_cast<LPSTR>( &pBuffer ), 0, nullptr );
    const std::string Message( pBuffer, Length );
    LocalFree( pBuffer );

    return TrimRight( Message );
  }

  std::string GetLastWin32Error()
  {
    return GetWin32Error( GetLastError() );
  }
#endif

  std::string Trim( const std::string_view str )
  {
    const auto Start = str.find_first_not_of( " \t\n\r\f\v" );
    if ( Start == std::string_view::npos )
    {
      return {};
    }

    const auto End = str.find_last_not_of( " \t\n\r\f\v" );
    return std::string { str.substr( Start, End - Start + 1 ) };
  }

  std::string TrimLeft( const std::string_view str )
  {
    const auto Start = str.find_first_not_of( " \t\n\r\f\v" );
    if ( Start == std::string_view::npos )
    {
      return {};
    }

    return std::string { str.substr( Start ) };
  }

  std::string TrimRight( const std::string_view str )
  {
    const auto End = str.find_last_not_of( " \t\n\r\f\v" );
    if ( End == std::string_view::npos )
    {
      return {};
    }

    return std::string { str.substr( 0, End + 1 ) };
  }
} // namespace Engine::Utility::String
