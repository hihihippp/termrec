#include "config.h"
#include "sys/error.h"

#ifdef WIN32
# include "sys/win32/winutils.h"
#else
# include "sys/unix/uutils.h"
#endif

#ifndef ARRAYSIZE
# define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif
