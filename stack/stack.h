#ifndef __STACK_H__
#define __STACK_H__

#define T	Stack_T
typedef struct Stack_T *Stack_T;

extern T Stack_new(void);
extern int Stack_empty(T stk);
extern void Stack_push(T stk, void* x);
extern void* Stack_pop(T stk);
extern void Stack_free(T* stk);

#undef T

/*
 * 本接口提供说明如下
 * 1. 不可检查的运行期错误(unchecked runtime error)
 * 2. 可检查的运行期错误(checked runtime error)
 * 3. 异常(exception)
 */
#endif