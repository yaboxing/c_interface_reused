#ifndef __SEM_H__
#define __SEM_H__

#define T Sem_T
typedef struct T {
    int count;
    void* queue;
} T;

#define LOCK(mutex) do{ Sem_T* _yymutex = &(mutex);Sem_wait(_yymutex);
#define END_LOCK    Sem_signal(_yymutex);} while(0)

extern void Sem_init(T* s, int count);
extern T* Sem_new(int count);
extern void Sem_wait(T* s);
extern void Sem_signal(T* s);

#undef T;
#endif
