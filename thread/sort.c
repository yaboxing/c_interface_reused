#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../assert/assert.h"
#include "../format/format.h"
#include "../mem/mem.h"
#include "thread.h"

struct args {
	int* a;
	int lb;
	int ub;
};

int cutoff = 10000;

int partition(int a[], int i, int j)
{
	int v;
	int k;
	int t;

	j++;
	k = i;
	v = a[k];
	while(i < j) {
		i++; while(a[i]<v && i<j) i++;
		j--; while(a[j]>v       ) j--;
	}
	t = a[k]; a[k] = a[j]; a[j] = t;

	return j;
}

#if 0
void quick(int a[], int lb, int ub)
{
	if(lb < ub) {
		int k = partition(a, lb, ub);
		quick(a, lb, k-1);
		quick(a, k+1, ub);
	}
}

void sort(int* x, int n, int argc, char* argvp[])
{
	quick(x, 0, n-1);
}
#endif


int quick(void* cl)
{
	struct args* p = (struct args*)cl;
	int lb = p->lb;
	int ub = p->ub;

	if(lb < ub){
		int k = partition(p->a, lb, ub);
		// quick
		p->lb = lb;
		p->ub = k - 1;
		if(k-lb > cutoff) {
			Thread_T t;
			t = Thread_new(quick, (void*)p, sizeof(*p), NULL);
			Fmt_print("thread %p sorted %d..%d\n", t, lb, k-1);
		} else {
			quick((void*)p);
		}
		p->lb = k + 1;
		p->ub = ub;
		if(ub-k > cutoff) {
			Thread_T t;
			t = Thread_new(quick, (void*)p, sizeof(*p), NULL);
			Fmt_print("thread %p sorted %d..%d\n", t, k+1, ub);
		} else {
			quick((void*)p);
		}
	}
}

void sort(int* x, int n, int argc, char* argv[])
{
	struct args args;

	if(argc >= 3){
		cutoff = atoi(argv[2]);
	}
	args.a  = x;
	args.lb = 0;
	args.ub = n - 1;
	quick(&args);
	Thread_join(NULL);
}

int main(int argc, char* argv[])
{
	int i;
	int n = 100000;
	int* x;
	int preempt;

	preempt = Thread_init(1, NULL);
	assert(preempt == 1);
	if(argc >=2){
		n = atoi(argv[1]);
	}
	x = CALLOC(n, sizeof(int));
	srand(time(NULL));
	for(i=1; i<n; i++){
		if(x[i] < x[i-1]) {
			break;
		}
	}
	assert(i == n);
	Thread_exit(EXIT_SUCCESS);

	return EXIT_SUCCESS;
}
