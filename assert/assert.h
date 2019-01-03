#ifndef __ASSERT_H__
#define __ASSERT_H__

#undef assert
#ifdef NDEBUG

#define assert(e)   ((void)0)

#else

#include "../except/except.h"
extern void assert(int e);
#define assert(e)   ((void)((e)||(RAISE(Assert_Failed) ,0)))

#endif

extern const Except_T Assert_Failed;

#endif
