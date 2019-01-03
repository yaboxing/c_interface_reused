#include "ring.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../assert/assert.h"
#include "../mem/mem.h"

#define T Ring_T

struct T {
    struct node {
        struct node* llink;
        struct node* rlink;
        void* value;
    }* head;
    int length;
};

T Ring_new(void)
{
    T ring;

    NEW0(ring);
    ring->head = NULL;

    return ring;
}

T Ring_ring(void* x, ...)
{
    va_list ap;
    T ring = Ring_new();

    va_start(ap, x);
    for (; x; x = va_arg(ap, void*)) {
        Ring_addhi(ring, x);
    }
    va_end(ap);

    return ring;
}

void Ring_free(T* ring)
{
    struct node* p;
    struct node* q;
    assert(ring && *ring);

    if ((p = (*ring)->head) != NULL) {
        int n = (*ring)->length;
        for (; n-- > 0; p = q) {
            q = p->llink;
            FREE(p);
        }
    }
    FREE(*ring);
}

int Ring_length(T ring)
{
    assert(ring);
    return ring->length;
}

void* Ring_get(T ring, int i)
{
    struct node* q;

    assert(ring);
    assert(i >= 0 && i < ring->length);

    // q <- ith node
    int n;
    q = ring->head;
    if (i <= ring->length / 2) {
        for (n = i; n-- > 0;) {
            q = q->rlink; // right
        }
    } else {
        for (n = ring->length - i; n-- > 0;) {
            q = q->llink; // left
        }
    }

    return q->value;
}

void* Ring_put(T ring, int i, void* x)
{
    struct node* q;
    void* prev;

    assert(ring);
    assert(i >= 0 && i < ring->length);

    // q <- ith node
    int n;
    q = ring->head;
    if (i <= ring->length / 2) {
        for (n = i; n-- > 0;) {
            q = q->rlink; // right
        }
    } else {
        for (n = ring->length - i; n-- > 0;) {
            q = q->llink; // left
        }
    }
    prev = q->value;
    q->value = x;

    return prev;
}

void* Ring_add(T ring, int pos, void* x)
{
    assert(ring);
    assert(pos >= -ring->length && pos <= ring->length + 1);

    if (pos == 1 || pos == -ring->length) {
        return Ring_addlo(ring, x);
    } else if (pos == 0 || pos == ring->length + 1) {
        return Ring_addhi(ring, x);
    }

    struct node* p;
    struct node* q;
    int i = pos < 0 ? pos + ring->length : pos - 1;

    // q <- ith node
    int n;
    q = ring->head;
    if (i <= ring->length / 2) {
        for (n = i; n-- > 0;) {
            q = q->rlink; // right
        }
    } else {
        for (n = ring->length - i; n-- > 0;) {
            q = q->llink; // left
        }
    }

    NEW(p);
    p->llink = q->llink; // add p into q's left
    q->llink->rlink = p;
    p->rlink = q;
    q->llink = p;
    ring->length++;

    return p->value = x;
}


void* Ring_addlo(T ring, void* x)
{
    assert(ring);

    // add and rotate
    Ring_addhi(ring, x);
    ring->head = ring->head->llink;

    return x;
}

void* Ring_addhi(T ring, void* x)
{
    struct node* p;
    struct node* q;

    assert(ring);

    NEW(p);
    if ((q = ring->head) != NULL) {
        p->llink = q->llink; // add p into q's left
        q->llink->rlink = p;
        p->rlink = q;
        q->llink = p;
    } else {
        ring->head = p->llink = p->rlink = p; // new node
    }

    ring->length++;

    return p->value = x;
}

void* Ring_remove(T ring, int i)
{
    void* x;
    struct node* q;

    assert(ring);
    assert(ring->length > 0);
    assert(i >= 0 && i < ring->length);

    // q <- ith node
    int n;
    q = ring->head;
    if (i <= ring->length / 2) {
        for (n = i; n-- > 0;) {
            q = q->rlink; // right
        }
    } else {
        for (n = ring->length - i; n-- > 0;) {
            q = q->llink; // left
        }
    }

    if (i == 0) {
        ring->head = ring->head->rlink;
    }
    x = q->value;

    // delete node q
    q->llink->rlink = q->rlink;
    q->rlink->llink = q->llink;
    FREE(q);
    if (--ring->length == 0) {
        ring->head = NULL;
    }

    return x;
}

void* Ring_remlo(T ring)
{
    assert(ring);
    assert(ring->length > 0);

    // rotate and remove
    ring->head = ring->head->rlink;

    return Ring_remhi(ring);
}

void* Ring_remhi(T ring)
{
    void* x;
    struct node* q;

    assert(ring);
    assert(ring->length > 0);

    q = ring->head->llink;
    x = q->value;

    // delete node q
    q->llink->rlink = q->rlink;
    q->rlink->llink = q->llink;
    FREE(q);
    if (--ring->length == 0) {
        ring->head = NULL;
    }

    return x;
}

void Ring_rotate(T ring, int n)
{
    struct node* q;
    int i;

    assert(ring);
    assert(n >= -ring->length && n <= ring->length);

    i = n >= 0 ? n % ring->length : n + ring->length;

    // q <- ith node
    int j;
    q = ring->head;
    if (i <= ring->length / 2) {
        for (j = i; j-- > 0;) {
            q = q->rlink; // right
        }
    } else {
        for (j = ring->length - i; j-- > 0;) {
            q = q->llink; // left
        }
    }

    ring->head = q;
}

