#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include <setjmp.h>

/*
1. 用户错误 -- 不可避免
2. 运行错误 -- 不在意料之中,程序漏洞,尽量避免
3. 异常 -- 需要程序处理,并自动恢复
*/

#define T   Except_T
typedef struct T {
    char* reason;
} T;

typedef struct Except_Frame Except_Frame;

struct Except_Frame {
    Except_Frame* prev;
    jmp_buf env;
    const char* file;
    int line;
    const T* exception;
};

enum {Except_entered = 0, Except_raised,
      Except_handled, Except_finalized
     };

extern Except_Frame* Except_stack;
extern void Except_raise(const T* e, const char* file, int line);

#define RAISE(e)    Except_raise(&(e), __FILE__, __LINE__)
#define RERAISE     Except_raise(Except_Frame.exceptions,   \
                                 Except_Frame.file,         \
                                 Except_Frame.line)
#define RETURN      switch() default: return

#define TRY do{                                 \
    volatile int Except_flag;                   \
    Except_Frame Except_fram;                   \
    Except_fram.prev = &Except_fram;            \
    Except_flag = setjmp(Except_fram.env);      \
    if(Except_flag == Except_entered){

#define EXCEPT(e)                               \
        if(Except_flag == Except_entered){      \
            Except_stack = Except_stack->prev;  \
        }                                       \
    } else if(Except_fram.exception == &(e)){   \
        Except_flag = Except_handled;

#define ELSE                                    \
        if(Except_flag == Except_entered){      \
            Except_stack = Except_stack->prev;  \
        }                                       \
    } else {                                    \
        Except_flag = Except_handled;           \

#define FINALLY                                 \
        if(Except_flag == Except_entered){      \
            Except_stack = Except_stack->prev;  \
        }                                       \
    }{                                          \
        if(Except_flag == Except_entered) {     \
            Except_flag = Except_finalized;     \
        }

#define END_TRY                                 \
        if(Except_flag == Except_entered){      \
            Except_stack = Except_stack->prev;  \
        }                                       \
    }                                           \
    if(Except_flag == Except_raised){           \
        RERAISE;                                \
    }                                           \
}while(0)

#undef T
#endif