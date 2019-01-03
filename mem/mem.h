#ifndef __MEM_H__
#define __MEM_H__

#include "../except/except.h"

extern const Except_T Mem_Failed;

extern void* Mem_alloc(long nbytes, const char* file, int line);
extern void* Mem_calloc(long count, long nbytes, const char* file, int line);

#define ALLOC(nbytes) \
    Mem_alloc((nbytes), __FILE__, __LINE__)

#define CALLOC(cont, nbytes) \
    Mem_calloc((cont), (nbytes), __FILE__, __LINE__)

#define NEW(p)  ((p) = ALLOC((long)sizeof *(p)))
#define NEW0(p) ((p) = CALLOC(1, (long)sizeof *(p)))

extern void Mem_free(void* ptr, const char* file, int line);

#define FREE(p) ((void)(Mem_free((p), __FILE__, __LINE__), (p)=0))

extern void* Mem_resize(void* p, long nbytes, const char* file, int line);

#define RESIZE(p, nbytes)   ((p) = Mem_resize((p), (nbytes), __FILE__, __LINE__))

#endif