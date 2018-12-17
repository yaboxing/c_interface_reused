#include <stdlib.h>
#include <stdarg.h>
#include "seq.h"
#include "../assert/assert.h"
#include "../mem/mem.h"
#include "../array/arrayrep.h"
#include "../array/array.h"
#include "../arith/arith.h"

#define T Seq_T

struct T{
	struct Array_T array;
	int length;
	int head;
};

#define SEQ(i) ((void**)seq->array.array)[(seq->head+(i))%(seq->array.length)]

T Seq_new(int hint)
{
	T seq;
	assert(hint >=0);
	NEW0(seq);
	hint = !hint? hint : 16;
	ArrayRep_init(&seq->array, 
				  hint, 
				  sizeof(void*), 
				  ALLOC(hint*sizeof(void*)));
	return seq;
}

T Seq_seq(void* x, ...)
{
	va_list ap;
	T seq = Seq_new(0);

	va_start(ap, x);
	for(; x; x=va_arg(ap, void*)){
		Seq_addhi(seq, x);
	}
	va_end(ap);

	return seq;
}

void Seq_free(T* seq)
{
	assert(seq && *seq);
	assert((void*)*seq == (void*)&(*seq)->array);

	Array_free((Array_T*)seq);
}

int Seq_length(T* seq)
{
	assert(seq);
	return seq->length;
}

void* Seq_get(T seq, int i)
{
	assert(seq);
	assert(i>=0 && i<seq->length);
	return SEQ(i);
}

void* Seq_put(T seq, int i, void* x)
{
	void* prev;

	assert(seq);
	assert(i>=0 && i<seq->length);

	// prev = a[i]; a[i] = x; return prev;
	prev = SEQ(i);
	SEQ(i) = x;

	return prev;
}

static void expend(T seq)
{
	int n = seq->array.length;

	Array_resize(&seq->array, n<<1);
	if(seq->head > 0){
		void** old = &((void**)seq->array.array)[seq->length];
		memcpy(old+n, old, (n-seq->head)*sizeof(void*));
		seq->head += n;
	}
}

void* Seq_addlo(T seq, void* x)
{
	int i = 0;
	assert(seq);
	if(seq->length==seq->array.length){
		expend(seq);
	}
	if(--seq->head < 0){
		// seq->head = seq->array.length - 1;
		seq->head = Arith_mod(seq->head-1, seq->array.length);
	}
	seq->length++;
	return SEQ(i) = x;
}

void* Seq_addhi(T seq, void* x)
{
	int i;
	assert(seq);
	if(seq->length == seq->array.length){
		expend(seq);
	}
	i = seq->length++;

	return SEQ(i) = x;
}

void* Seq_remlo(T seq)
{
	int i = 0;
	void* x;

	assert(seq);
	assert(seq->length > 0);
	x = SEQ(i);
	seq->head = (seq->head+1) % seq->array->array.length;
	--seq->length;

	return x;
}

void* Seq_remhi(T seq)
{
	int i;

	assert(seq);
	assert(seq->length > 0);
	i = --seq->length;

	return SEQ(i);
}

