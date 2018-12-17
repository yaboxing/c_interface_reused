#include "stack.h"
#include "../mem/mem.h"

#define T	Stack_T

struct T{
	int count;
	struct elem{
		void* x;
		struct elem* link;
	}*head;
};

//#define NEW(x) (x)=malloc(sizeof(struct T))

T Stack_new(void)
{
	T stk;

	NEW(stk);
	stk->count = 0;
	stk->head = NULL;

	return stk;
}

int Stack_empty(T stk)
{
	assert(stk); // assert(e) 确保表达式 e 为真

	return stk->count == 0;

}

void Stack_push(T stk, void* x)
{
	struct elem* t;
	
	assert(stk);
	NEW(t);
	t->x = x;
	t->link = stk->head;
	stk->head = t;
	stk->count++;
}

void* Stack_pop(T stk)
{
	void* x;
	struct elem* t;

	assert(stk);
	assert(stk->count > 0);

	t = stk->head;
	stk->head = t->link;
	stk->count--;
	x = t->x;
	FREE(t);
	t = NULL;
	return x;
}

void Stack_free(T* stk)
{
	struct elem* t;
	struct elem* u;

	assert(stk && *stk);

	for(t=(*stk)->head; t; t=u){
		u = t->link;
		FREE(t);
		t = NULL;
	}

	FREE(*stk);
}

int main(int argc, char* argv[])
{
	Stack_T names = Stack_new();

	Stack_free(&names);
	return 0;
}
