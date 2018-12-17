#ifndef __GETWORD_H__
#define __GETWORD_H__

#include <stdio.h>

extern int getword(FILE* fp, char* buf, int size,
				int first(int c), int rest(int c));

extern int first(int c);
extern int rest(int c);
extern wf(char * name, FILE * fp);
int compare(const void* x, const void* y);


#endif
