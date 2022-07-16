#pragma once

// enable memory leak checking
#define _CRT_DEBUG_MAP_ALLOC
#include <crtdbg.h> // c runtime debug

/* paste in include list in main() file

#if defined(_WIN32)
#include "win32_leak_check.h"
#endif

*/

/* paste in main() to check for leaks in debug

#if defined(_WIN32) && defined(_DEBUG)
int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;   // check block integrity
dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF; // don't recycle memory
dbgFlags |= _CRTDBG_LEAK_CHECK_DF;     // leak report on exit
_CrtSetDbgFlag(dbgFlags);
#endif

*/