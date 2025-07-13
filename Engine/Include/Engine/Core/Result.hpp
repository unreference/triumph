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

#include <variant>
#include <string>

#include <Engine/Core/Macro.hpp>

namespace Engine
{
  template <typename T, typename E = std::string> class Result
  {
    DISALLOW_COPY( Result );
    DISALLOW_MOVE( Result );

  public:
    Result( T && value )
      : m_Data( std::forward<T>( value ) )
    {
    }

    Result( const T & value )
      : m_Data( value )
    {
    }

    Result( E && error )
      : m_Data( std::forward<E>( error ) )
    {
    }

    Result( const E & error )
      : m_Data( error )
    {
    }

    [[nodiscard]] bool IsValid() const
    {
      return std::holds_alternative<T>( m_Data );
    }

    explicit operator bool() const
    {
      return IsValid();
    }

    [[nodiscard]] const T & GetValue() const
    {
      return std::get<T>( m_Data );
    }

    T & GetValue()
    {
      return std::get<T>( m_Data );
    }

    [[nodiscard]] const E & GetError() const
    {
      return std::get<E>( m_Data );
    }

    E & GetError()
    {
      return std::get<E>( m_Data );
    }

    const T & operator*() const
    {
      return GetValue();
    }

    T & operator*()
    {
      return GetValue();
    }

  private:
    std::variant<T, E> m_Data = {};
  };
} // namespace Engine
