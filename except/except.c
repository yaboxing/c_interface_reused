#include "except.h"
#include "../assert/assert.h"

Except_Fram* Except_stack = NULL;

void Except_raise(const T* e, const char* file, int line)
{
	Except_Fram* p = Except_stack;
	
	assert(e);
	
	if(p == NULL){
		fprintf(stderr, "Uncaught exception");
		if(e->reason){
			fprintf(stderr, "%s", e->reason);
		} else {
			fprintf(stderr, " ar 0x%p", e);
		}
		
		if(file && line>0){
			fprintf(stderr, "raised at %s:%d\n", file, line);
		}
		
		fprintf(stderr, "aborting...\n");
		fflush(stderr);
		abort();
	}
	
	p->exception = e;
	p->file = file;
	p->line = line;
	
	longjmp(p->env, Except_raised);
}

int main(int argc, char* argv[])
{
TRY
	edit(argc, argv);
ELSE
	fprintf(stderr,
		"An internal error has occured from there is "
		"no recovery.\nPlease report this error to "
		"Technical Support at 800-777-1234.\nNote the "
		"follow messing, which will help our support "
		"staff\nfind the cause of this error.\n\n");
RERAISE;
END_TRY;
	
	return EXIT_SUCCESS;
}