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

#define DISALLOW_COPY( type )                                                       \
public:                                                                             \
  type( const type & )             = delete;                                        \
  type & operator=( const type & ) = delete;                                        \
                                                                                    \
private:                                                                            \
  static_assert( true, "" )

#define DISALLOW_MOVE( type )                                                       \
public:                                                                             \
  type( type && )             = delete;                                             \
  type & operator=( type && ) = delete;                                             \
                                                                                    \
private:                                                                            \
  static_assert( true, "" )
