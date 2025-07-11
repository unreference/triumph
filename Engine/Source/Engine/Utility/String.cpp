#if defined( PLATFORM_WIN32 )
#include <Windows.h>
#elif defined( PLATFORM_UNIX )
#include <codecvt>
#include <locale>
#endif

#include "Engine/Utility/String.hpp"

namespace Engine::Utility::String
{
  std::string ToUtf8( std::wstring_view utf16 )
  {
    if ( utf16.empty() )
    {
      return {};
    }

#if defined( PLATFORM_WIN32 )
    const auto Length = WideCharToMultiByte( CP_UTF8, 0, utf16.data(),
                                             static_cast<i32>( utf16.length() ),
                                             nullptr, 0, nullptr, nullptr );
    if ( Length == 0 )
    {
      return {};
    }

    std::string result( Length, 0 );
    WideCharToMultiByte( CP_UTF8, 0, utf16.data(),
                         static_cast<i32>( utf16.length() ), result.data(),
                         Length, nullptr, nullptr );
    return result;
#else
    std::wstring_convert<std::codecvt<wchar_t>> convert;
    return convert.to_bytes( utf16.data(), utf16.data() + utf16.length() );
#endif
  }

  std::string ToUtf8( const std::wstring & utf16 )
  {
    return ToUtf8( std::wstring_view { utf16 } );
  }

  std::string ToUtf8( const wchar_t * utf16 )
  {
    return utf16 ? ToUtf8( std::wstring_view { utf16 } ) : std::string {};
  }

  void ToUtf8( const std::wstring_view utf16, std::string & utf8 )
  {
    utf8 = ToUtf8( utf16 );
  }

  std::wstring ToUtf16( std::string_view utf8 )
  {
    if ( utf8.empty() )
    {
      return {};
    }

#if defined( PLATFORM_WIN32 )
    const auto Length = MultiByteToWideChar(
      CP_UTF8, 0, utf8.data(), static_cast<i32>( utf8.length() ), nullptr, 0 );
    if ( Length == 0 )
    {
      return {};
    }

    std::wstring result( Length, 0 );
    MultiByteToWideChar( CP_UTF8, 0, utf8.data(),
                         static_cast<i32>( utf8.length() ), result.data(),
                         Length );
    return result;
#else
    std::wstring_convert<std::codecvt<wchar_t>> convert;
    return convert.from_bytes( utf8.data(), utf8.data() + utf8.length() );
#endif
  }

  std::wstring ToUtf16( const std::string & utf8 )
  {
    return ToUtf16( std::string_view { utf8 } );
  }

  std::wstring ToUtf16( const char * utf8 )
  {
    return utf8 ? ToUtf16( std::string_view { utf8 } ) : std::wstring {};
  }

  void ToUtf16( const std::string_view utf8, std::wstring & utf16 )
  {
    utf16 = ToUtf16( utf8 );
  }

#if defined( PLATFORM_WIN32 )
  std::string GetWin32Error( const u32 errorCode )
  {
    LPSTR             pBuffer = nullptr;
    const std::size_t Size    = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, errorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
      reinterpret_cast<LPSTR>( &pBuffer ), 0, nullptr );

    const std::string Message( pBuffer, Size );
    LocalFree( pBuffer );

    return TrimRight( Message );
  }

  std::string GetLastWin32Error()
  {
    return GetWin32Error( GetLastError() );
  }
#endif

  bool IsUtf8( const std::string_view string )
  {
    for ( std::size_t i = 0; i < string.length(); /**/ )
    {
      if ( const auto Char1 = static_cast<u8>( string.at( i ) ); Char1 < 0x80 )
      {
        // ASCII
        i += 1;
      }
      else if ( Char1 >> 5 == 0b110 )
      {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if ( i + 1 >= string.length() )
        {
          return false;
        }

        const auto Char2 = static_cast<u8>( string.at( i + 1 ) );
        if ( ( Char2 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        // Reject overlong (i.e.: U+007F encoded as 0xC1 0xBF)
        if ( const auto Code = ( Char1 & 0x1F ) << 6 | Char2 & 0x3F;
             Code < 0x80 )
        {
          return false;
        }

        i += 2;
      }
      else if ( Char1 >> 4 == 0b1110 )
      {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if ( i + 2 >= string.length() )
        {
          return false;
        }

        const auto Char2 = static_cast<u8>( string.at( i + 1 ) );
        const auto Char3 = static_cast<u8>( string.at( i + 2 ) );
        if ( ( Char2 & 0xC0 ) != 0x80 || ( Char3 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        const auto Code =
          ( Char1 & 0x0F ) << 12 | ( Char2 & 0x3F ) << 6 | Char3 & 0x3F;

        // Reject overlong or surrogates (0xD800-0xDFFF)
        if ( Code < 0x800 || ( Code >= 0xD800 && Code <= 0xDFFF ) )
        {
          return false;
        }

        i += 3;
      }
      else if ( Char1 >> 3 == 0b11110 )
      {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if ( i + 3 >= string.length() )
        {
          return false;
        }

        const auto Char2 = static_cast<u8>( string.at( i + 1 ) );
        const auto Char3 = static_cast<u8>( string.at( i + 2 ) );
        const auto Char4 = static_cast<u8>( string.at( i + 3 ) );
        if ( ( Char2 & 0xC0 ) != 0x80 || ( Char3 & 0xC0 ) != 0x80 ||
             ( Char4 & 0xC0 ) != 0x80 )
        {
          return false;
        }

        const auto Code = ( Char2 & 0x07 ) << 18 | ( Char2 & 0x3F ) << 12 |
                          ( Char3 & 0x3F ) << 6 | Char4 & 0x3F;

        // Reject overlong or > U+10FFFF
        if ( Code < 0x10000 || Code > 0x10FFFF )
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

  bool IsUtf16( const std::wstring_view string )
  {
    for ( std::size_t i = 0; i < string.length(); /**/ )
    {
      if ( const auto Char1 = string.at( i ); Char1 < 0xD800 || Char1 > 0xDFFF )
      {
        // Plain BMP
        i += 1;
      }
      else if ( Char1 <= 0xDBFF )
      {
        // High surrogate
        if ( i + 1 >= string.length() )
        {
          return false;
        }

        // Low surrogate must be 0xD800..0xDFFF
        if ( const auto Char2 = string.at( i + 1 );
             Char2 < 0xDC00 || Char2 > 0xDFFF )
        {
          return false;
        }

        i += 2;
      }
      else
      {
        // Low surrogate without a preceding high
        return false;
      }
    }

    return true;
  }

  std::string Trim( const std::string_view string )
  {
    const auto Start = string.find_first_not_of( " \t\n\r\f\v" );
    if ( Start == std::string_view::npos )
    {
      return {};
    }

    const auto End = string.find_last_not_of( " \t\n\r\f\v" );
    return std::string { string.substr( Start, End - Start + 1 ) };
  }

  std::string TrimLeft( const std::string_view string )
  {
    const auto Start = string.find_first_not_of( " \t\n\r\f\v" );
    if ( Start == std::string_view::npos )
    {
      return {};
    }

    return std::string { string.substr( Start ) };
  }

  std::string TrimRight( const std::string_view string )
  {
    const auto End = string.find_last_not_of( " \t\n\r\f\v" );
    if ( End == std::string_view::npos )
    {
      return {};
    }

    return std::string { string.substr( 0, End + 1 ) };
  }

} // namespace Engine::Utility::String
