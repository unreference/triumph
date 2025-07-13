/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include <Engine/Utility/Logger.hpp>

#include "Main.hpp"

int main()
{
  try
  {
    const auto pApp = Engine::Core::Create();
    pApp->Run();
    delete pApp;
    return 0;
  }
  catch ( const std::exception & E )
  {
    LOG_FATAL( E.what() );
  }
}
