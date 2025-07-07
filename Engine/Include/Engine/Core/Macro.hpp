#pragma once

#define DISALLOW_COPY( type )                                                  \
public:                                                                        \
  type( const type & )             = delete;                                   \
  type & operator=( const type & ) = delete;                                   \
                                                                               \
private:                                                                       \
  static_assert( true, "" )

#define DISALLOW_MOVE( type )                                                  \
public:                                                                        \
  type( type && )             = delete;                                        \
  type & operator=( type && ) = delete;                                        \
                                                                               \
private:                                                                       \
  static_assert( true, "" )
