#include "assert.h"

const Except_T	Assert_Failed = {"Assert failed"};

void (assert)(int e){
	assert(e);
}