#include <stdio.h>
#include <stdlib.h>
#include "../assert/assert.h"
#include "../format/format.h"
#include "thread.h"
#include "sem.h"

#define NBUMP   30000

struct args {
    SemT* mutex;
    int* ip;
};

int unsafe(void* cl)
{
    int i;
    int* ip = (int*)cl;
    for (i = 0; i < NBUMP; i++) {
        *ip += 1;
    }

    return EXIT_SUCCESS;
}

int safe(void* cl)
{
    int i;
    struct args* p = (struct args*)cl;
    for (i = 0; i < NBUMP; i++) {
        LOCK(*p->mutex)
        *p->ip += i;
        END_LOCK;
    }

    return EXIT_SUCCESS;
}

int n;
int main(int argc, char* argv[])
{
    int m = 5;
    int preempt;

    preempt = Thread_init(1, NULL);
    assert(preempt == 1);
    if (argc >= 2) {
        m = atoi(argv[1]);
    }
    n = 0;
    // increment n unsafely
    {
        int i;
        for (i = 0; i < m; i++) {
            Thread_new(unsafe, &n, 0, NULL);
        }
        Thread_join(NULL);
    }

    n = 0;
    // increment n safly
    Fmt_print("%d == %d\n", n, NBUMP * m);
    {
        int i;
        struct args args;
        Sem_T mutex;
        Sem_init(&mutex, 1);
        args.ip = &n;
        for (i = 0; i < m; i++) {
            Thread_new(safe, &args, sizeof(args), NULL);
        }
        Thread_join(NULL);
    }
    Fmt_print("%d == %d\n", n, NBUMP * m);
    Thread_exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
