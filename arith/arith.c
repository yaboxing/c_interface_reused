#include "arith.h"

int Arith_max(int x, int y)
{
	return x>y? x : y;
}

int Arith_min(int x, int y)
{
	return x<y? x : y;
}

int Arith_div(int x, int y)
{
	int ret;

	if(	(-13/5==-2) 	&& 
		((x<0)!=(y<0))	&&
		(x%y!=0)) {
		ret = x/y - 1;
	} else {
		ret = x/y;
	}

	return ret;
}

int Arith_mod(int x, int y)
{
	int ret;

	if(	(-13/5==-2) 	&& 
		((x<0)!=(y<0))	&&
		(x%y!=0)){
		ret = x%y + y;
	} else {
		ret = x%y;
	}

	return ret;
}

int Arith_ceiling(int x, int y)
{
	int ret;

	ret = Arith_div(x, y) + (x%y!=0);

	return ret;
}

int Arith_floot(int x, int y)
{
	int ret;

	ret = Arith_div(x, y);
	
	return ret;
}
