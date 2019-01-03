#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../assert/assert.h"
#include "../except/except.h"
#include "../mem/mem.h"

#define hash(p, t)  (((unsigned long)(p)>>3) & (sizeof((t))/sizeof((t)[0]-1))
#define NDECRIPTORS 512
#define NALLOC      4096

union align {
    int i;
    long l;
    long* lp;
    void* p;
    void (*fp)(void);
    float f;
    double d;
    long double ld;
};

static struct descriptor {
    struct descriptor* free;
    struct descriptor* link;
    const void* p;
    long size;
    const char* File;
    int line;
}* htab[2048];

static struct descriptor freelist = { &freelist };

static struct descriptor* find(const void* p)
{
    struct descriptor* bp = htab[hash(p, htab)];

    while (bp && bp->p != p) {
        bp = bp->link;
    }

    return bp;
}

void Mem_free(void* p, const char* file, int line)
{
    if (p) {
        struct descriptor* bp;
        if ((((unsigned long)p) % (sizeof(union align))) != 0 ||
            (bp = find(p)) == NULL ||
            bp->free) {
            Except_raise(&Assert_Failed, file, line);
        }
        bp->free = freelist.free;
        freelist.free = bp;
    }
}

void* Mem_resize(void* p, long nbytes, const char* file, int line)
{
    struct descriptor* bp;
    void* newp;

    assert(p);
    assert(nbytes > 0);
    if ((((unsigned long)p) % (sizeof(union align))) != 0 ||
        (bp = find(p)) == NULL ||
        bp->free) {
        Except_raise(&Assert_Failed, file, line);
    }
    newp = Mem_alloc(nbytes, file, line);
    memcpy(newp, p, nbytes < bp->size ? nbytes : bp->size);
    Mem_free(p, file, line);

    return newp;
}

void* Mem_calloc(long count, long nbytes, const char* file, int line)
{
    void* p;

    assert(count > 0);
    assert(nbytes > 0);

    p = Mem_alloc(count * nbytes, file, line);
    memset(p, '\0', count * nbytes);

    return p;
}

static struct descriptor* dalloc(void* p, long size, const char* file, int line)
{
    static struct descriptor* avail;
    static int nleft;

    if (nleft <= 0) {
        avail = malloc(NDECRIPTORS * sizeof(*avail));
        if (avail == NULL) {
            return NULL;
        }
        nleft = NDECRIPTORS;
    }
    avail->p = p;
    avail->size = size;
    avail->File = file;
    avail->line = line;
    avail->free = avail->link = NULL;
    nleft--;

    return avail++;
}

void Mem_alloc(long nbytes, const char* file, int line)
{
    struct descriptor* bp;
    void* p;

    assert(nbytes > 0);

    nbytes = ((nbytes + sizeof(union align) - 1) /
              (sizeof(union align))) * (sizeof(union align));
    for (bp = freelist.free; bp; bp = bp->free) {
        if (bp->size > nbytes) {
            bp->size -= nbytes;
            p = (char*)bp->p + bp->size;
            if ((bp = dalloc(p, nbytes, file, line)) != NULL) {
                unsigned h = hash(p, htab);
                bp->link = htab[h];
                htab[h] = bp;
                return p;
            } else {
                if (file == NULL) {
                    RAISE(Mem_Failed);
                } else {
                    Except_raise(&Mem_Failed, file, line);
                }
            }
        }

        if (bp == &freelist) {
            struct descriptor* newp;
            if ((p = malloc(nbytes + NALLOC)) == NULL ||
                (newp = dalloc(p, nbytes + NALLOC, __FILE__, __LINE__)) == NULL) {
                if (file == NULL) {
                    RAISE(Mem_Failed);
                } else {
                    Except_raise(&Mem_Failed, file, line);
                }
            }
            newp->free = freelist.free;
            freelist.free = newp;
        }
    }

    assert(0);
    return NULL;
}
















