#include "mem.h"
#include "../except/except.h"
#include "../assert/assert.h"
#include <stdlib.h>

const Except_T Mem_Failed = {"Allocation Failed"};

void* Mem_alloc(long nbytes, const char* file, int line)
{
    void* p;

    assert(nbytes > 0);

    p = malloc(nbytes);
    if (p == NULL) {
        if (file == NULL) {
            RAISE(Mem_Failed);
        } else {
            Except_raise(&Mem_Failed, file, line);
        }
    }

    return p;
}

void* Mem_calloc(long count, long nbytes, const char* file, int line)
{
    void* p;

    assert(count > 0);
    assert(nbytes > 0);

    p = calloc(count, nbytes);
    if (p == NULL) {
        if (file == NULL) {
            RAISE(Mem_Failed);
        } else {
            Except_raise(&Mem_Failed, file, line);
        }
    }

    return p;
}

void Mem_free(void* ptr, const char* file, int line)
{
    if (ptr) {
        free(ptr);
    }
}

void* Mem_resize(void* p, long nbytes, const char* file, int line)
{
    assert(p);
    assert(nbytes > 0);
    p = realloc(p, nbytes);
    if (p == NULL) {
        if (file == NULL) {
            RAISE(Mem_Failed);
        } else {
            Except_raise(&Mem_Failed, file, line);
        }
    }

    return p;
}