#include "list.h"
#include "../mem/mem.h"

void mkatom(void** x, void* cl)
{
	char** str = (char**)x;
	FILE* fp = cl;

	*str = Atom_string(*str);
	fprintf(fp, "%s\n", *str);
}

void applyFree(void** p, void* cl)
{
	FREE(*p);
}

T List_list(void * x, ...)
{
	va_list ap;
	T list;
	T* p = &list;

	va_start(ap, x);
	for(; x; x=va_arg(ap, void*)){
		NEW(*p);
		(*p)->first = x;
		p = &(*p)->rest;
	}
	*p = NULL;
	va_end(ap);

	return list;
}

T List_append(T list, T tail)
{
	T* p = &list;

	while(*p){
		p = &(*p)->rest;
	}

	*p = tail;

	return list;
}


T List_copy(T list)
{
	T head;
	T* p = &head;

	for(; list; list=list->rest){
		NEW(*p);
		(*p)->first = list->first;
		p = &(*p)->rest;
	}
	*p = NULL;

	return head;
}

T List_push(T list, void * x)
{
	T p;

	NEW(p);
	p->first = x;
	p->rest = list;
	return p;
}

T List_pop(T list, void * * x)
{
	if(list){
		T head = list->rest;
		if(x){
			*x = list->first;
		}
		FREE(list);

		return head;
	}

	return list;
}

T List_reverse(T list)
{
	T head = NULL;
	T next;

	for(; list; list=next){
		next = list->rest;
		list->rest = head;
		head = list;
	}

	return head;
}


int List_length(T list)
{
	int n;

	for(n=0; list; list=list->rest){
		n++;
	}

	return n;
}

void List_free(T * list)
{
	T* next;

	for(; *list; *list=next;){
		next = (*list)->rest;
		FREE(*list);
	}
}

void List_map(T list, void apply(void * * x, void * cl), void * cl)
{
	assert(apply);

	for(; list; list=list->rest){
		apply(&list->first, cl);
	}
}

void** List_toArray(T list, void * end)
{
	int i;
	int n = List_length(list);
	void** array = ALLOC((n+1) * sizeof(*array));

	for(i=0; i<n; i++){
		array[i] = list->first;
		list = list->rest;
	}

	array[i] = end;

	return array;
}