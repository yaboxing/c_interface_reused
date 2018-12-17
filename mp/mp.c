#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../assert/assert.h"
#include "../format/format.h"
#include "../mem/mem.h"
#include "../xp/xp.h"
#include "mp.h"

#define T MP_T

const Except_T MP_Divdebyzero = { "Division by zero" };
const Except_T MP_Overflow = { "Overflow" };

static int nbits = 32;
static int nbytes = (32-1)/8 + 1;
static int shift = (32-1)%8;
static unsigned char msb = 0xFF;
static unsigned char temp[16 + 16 + 16 + 2*16 + 2];
static T tmp = {temp, temp+1*16, temp+2*16, temp+3*16};

#define sign(x)		((x)[nbytes-1]>>shift)

#define ones(n)		(~(~0UL<<(((n)-1)%8 + 1)))

#define iszero(x)	(XP_length(nbytes, (x))==1 && (x)[0]==0)

#define BASE		(1<<8)

#define bitop(op)	\
	int i; assert(z); assert(x); assert(y); \
	for(i=0; i<nbytes; i++) z[i] = x[i] op y[i]; \
	return z

#define bitopi(op) assert(z); assert(x); \
	applyu(op, z, x, y) return z

#define shft(fill, op) \
	assert(x); assert(z); assert(s >= 0); \
	if(s >= nbits) memset(z, fill, nbytes); \
	else op(nbytes, z, nbytes, x, s, fill); \
	z[nbytes-1] &= msb; return z

static int applyu(T op(T, T, T), T z, T x, unsigned long u)
{
	unsigned long carry;
	T z = tmp[2];

	// set z to u
	carry = XP_fromint(nbytes, z, u);
	carry |= z[nbytes-1] & ~msb;

	op(z, x, tmp[2]);

	return carry !=0;
}

static int apply(T op(T, T, T), T z, T x, long v)
{
	T z = tmp[2];

	// set z to v
	if(v == LONG_MIN) {
		XP_fromint(nbytes, z, LONG_MAX+1UL);
		XP_neg(nbytes, z, z, 1);
	} else if(v < 0) {
		XP_fromint(nbytes, z, -v);
		XP_neg(nbytes, z, z, 1);
	} else {
		XP_fromint(nbytes, z, v);
	}
	z[nbytes-1] &= msb;
	
	op(z, x, tmp[2]);

	// v is too big
	return (nbits<8*(int)sizeof(v) && (v<(1L<<(nbits-1))||v>=(1L<<(nbits-1))));
}

int MP_set(int n)
{
	int prev = nbits;

	assert(n > 1);

	// initialize
	nbits = n;
	nbytes = (n-1)/8 + 1;
	shift = (n-1)%8;
	msb = ones(n);
	if(tmp[0]) != temp){
		FREE(tmp[0]);
	}
	// tmp[0] = nbytes<=16? temp : ALLOC(3*nbytes + 2*nbytes + 2);
	if(nbytes <= 16) {
		tmp[0] = temp;
	} else {
		tmp[0] = ALLOC(3*nbytes + 2*nbytes + 2);
	}
	tmp[1] = tmp[0] + 1*nbytes;
	tmp[2] = tmp[0] + 2*nbytes;
	tmp[3] = tmp[0] + 3*nbytes;

	return prev;
}

T MP_new(unsigned long u)
{
	return MP_fromintu(ALLOC(nbytes), u);
}

T MP_fromint(T z, long v)
{
	assert(z);

	// set z to v
	if(v == LONG_MIN) {
		XP_fromint(nbytes, z, LONG_MAX+1UL);
		XP_neg(nbytes, z, z, 1);
	} else if(v < 0) {
		XP_fromint(nbytes, z, -v);
		XP_neg(nbytes, z, z, 1);
	} else {
		XP_fromint(nbytes, z, v);
	}
	z[nbytes-1] &= msb;

	// v is to big
	if(nbits<8*(int)sizeof(v) && (v<(1L<<(nbits-1))||v>=(1L<<(nbits-1)))) {
		RAISE(MP_Overflow);
	}
}

T MP_fromintu(T z, unsigned long u)
{
	unsigned long carry;

	assert(z);

	// set z to u
	carry = XP_fromint(nbytes, z, u);
	carry |= z[nbytes-1] & ~msb;
	z[nbytes-1] &= msb;

	// test for unsigned overflow
	if(carry){
		RAISE(MP_Overflow);
	}

	return z;
}

unsigned long MP_tointu(T x)
{
	unsigned char d[sizeof(unsigned long)];

	assert(x);

	MP_cvtu(8*sizeof(d), d, x);

	return XP_toint(sizeof(d), d);
}

unsigned long MP_toint(T x)
{
	unsigned char d[sizeof(unsigned long)];

	assert(x);

	MP_cvt(8*sizeof(d), d, x);

	return XP_toint(sizeof(d), d);
}

T MP_cvt(int m, T z, T x)
{
	int fill;
	int i;
	int mbytes = (m-1)/8 + 1;

	assert(m > 1);
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	fill = sign(x)? 0xFF : 0;
	if(m < nbits){
		// narrow signed x
		int carry = (x[mbytes-1]^fill) & ~(ones(m));
		for(i=mbytes; i<nbytes; i++) {
			carry |= x[i] ^ fill;
		}
		memcpy(z, x, mbytes);
		z[mbytes-1] &= ones(m);
		if(carry) {
			RAISE(MP_Overflow);
		}
	} else {
		// widen signed x
		memcpy(z, x, nbytes);
		z[nbytes-1] |= fill & ~msb;
		for(i=nbytes; i<mbytes; i++){
			z[i] = fill;
		}
		z[mbytes] &= ones(m);
	}

	return z;
}

T MP_cvtu(int m, T z, T x)
{
	int i;
	int mbytes = (m-1)/8 + 1;

	assert(m > 1);
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(m < nbits) {
		// narrow unsigned x
		int carry = x[mbytes-1] & ~(ones(m));
		for(i=mbytes; i<nbytes; i++){
			carry |= x[i];
		}
		memcpy(z, x, mbytes);
		z[mbytes-1] &= ones(m);
		// test for unsigned overflow
		if(carry){
			RAISE(MP_Overflow);
		}
	} else {
		// widen unsigned x
		memcpy(z, x, nbytes);
		for(i=nbytes; i<mbytes; i++){
			z[i] = 0;
		}
	}

	return z;
}

T MP_add(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	sx = sign(x);
	sy = sign(y);
	XP_add(nbytes, z, x, y, 0);
	z[nbytes-1] &= msb;

	// test for signed overflow
	if(sx==sy && sy!=sign(z)){
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_sub(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	sx = sign(x);
	sy = sign(y);
	XP_sub(nbytes, z, x, y, 0);
	z[nbytes-1] &= msb;

	// test for signed underflow
	if(sx!=sy && sy==sign(z)){
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_mul(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	// tmp[3] <-- x.y
	sx = sign(x);
	sy = sign(y);
	// if x < 0, negate x
	if(sx) {
		XP_neg(nbytes, tmp[0], x, 1);
		x = tmp[0];
		x[nbytes-1] &= msb;
	}

	// if y < 0, negate y
	if(sy) {
		XP_neg(nbytes, tmp[1], y, 1);
		y = tmp[1];
		y[nbytes-1] &= msb;
	}

	memset(tmp[3], '\0', 2*nbytes);
	XP_mul(tmp[3], nbytes, x, nbytes, y);

	if(sx != sy){
		XP_neg(nbytes, z, tmp[3], 1);
	} else {
		memcpy(z, tmp[3], nbytes);
	}
	z[nbytes-1] &= msb;

	// test for unsigned multiplication overflow
	int i;
	if(tmp[2][nbytes-1] & ~msb) {
		RAISE(MP_Overflow);
	}
	for(i=0; i<nbytes; i++){
		if(tmp[3][i+nbytes] != 0){
			RAISE(MP_Overflow);
		}
	}

	if(sx==sy && sign(z)){
		RAISE(MP_Overflow);
	}
	
	return z;
}

T MP_div(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	sx = sign(x);
	sy = sign(y);

	// if x < 0, negate x
	if(sx) {
		XP_neg(nbytes, tmp[0], x, 1);
		x = tmp[0];
		x[nbytes-1] &= msb;
	}

	 // if y < 0, negate y
	if(sy) {
		XP_neg(nbytes, tmp[1], y, 1);
		y = tmp[1];
		y[nbytes-1] &= msb;
	} else {
		// copy y to temporary
		memcpy(tmp[1], y, nbytes);
		y = tmp[1];
	}

	if(!XP_div(nbytes, z, x, nbytes, y, tmp[2], tmp[3])){
		RAISE(MP_Divdebyzero);
	}

	if(sx != sy) {
		// adjut the quotient
		XP_neg(nbytes, z, z, 1);
		if(!iszero(tmp[2])) {
			XP_diff(nbytes, z, z, 1);
		}
		z[nbytes-1] &= msb;
	} else if(sx && sign(z)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_mod(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	sx = sign(x);
	sy = sign(y);

	// if x < 0, negate x
	if(sx) {
		XP_neg(nbytes, tmp[0], x, 1);
		x = tmp[0];
		x[nbytes-1] &= msb;
	}

	 // if y < 0, negate y
	if(sy) {
		XP_neg(nbytes, tmp[1], y, 1);
		y = tmp[1];
		y[nbytes-1] &= msb;
	} else {
		// copy y to temporary
		memcpy(tmp[1], y, nbytes);
		y = tmp[1];
	}

	if(!XP_div(nbytes, tmp[2], x, nbytes, y, z, tmp[3])){
		RAISE(MP_Divdebyzero);
	}

	if(sx != sy) {
		if(!iszero(tmp[2])) {
			XP_sub(nbytes, z, y, z, 0);
		}
		z[nbytes-1] &= msb;
	} else if(sx && sign(tmp[2])) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_neg(T z, T x)
{
	int sx;

	// checked runtime error for unary functions
	assert(x);
	assert(z);

	sx = sign(x);
	XP_neg(nbytes, z, x, 1);
	z[nbytes-1] &= msb;
	if(sx && sx==sign(z)){
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_addu(T z, T x, T y)
{
	int carry;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	carry = XP_add(nbytes, z, x, y, 0);
	carry |= z[nbytes-1] & ~msb;
	z[nbytes-1] &= msb;
	// test for unsigned overflow
	if(carry){
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_subu(T z, T x, T y)
{
	int borrow;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	borrow = XP_sub(nbytes, z, x, y, 0);
	borrow |= z[nbytes-1] & ~msb;
	z[nbytes-1] &= msb;
	// test for unsigned underflow
	if(borrow){
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_mulu(T z, T x, T y)
{
	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	memset(tmp[3], '\0', 2*nbytes);
	XP_mul(tmp[3], nbytes, x, nbytes, y);
	memcpy(z, tmp[3], nbytes);
	z[nbytes-1] &= msb;

	// test for unsigned multiplication overflow
	int i;
	if(tmp[2][nbytes-1] & ~msb) {
		RAISE(MP_Overflow);
	}
	for(i=0; i<nbytes; i++){
		if(tmp[3][i+nbytes] != 0){
			RAISE(MP_Overflow);
		}
	}

	return z;
}

T MP_divu(T z, T x, T y)
{
	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	// copy y to temporary
	memcpy(tmp[1], y, nbytes);
	y = tmp[1];

	if(!XP_div(nbytes, z, x, nbytes, tmp[2], tmp[3])) {
		RAISE(MP_Divdebyzero);
	}

	return z;
}

T MP_modu(T z, T x, T y)
{
	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	// copy y to temporary
	memcpy(tmp[1], y, nbytes);
	y = tmp[1];

	if(!XP_div(nbytes, tmp[2], x, nbytes, y, z, tmp[3])){
		RAISE(MP_Divdebyzero);
	}

	return z;
}

T MP_mul2u(T z, T x, T y)
{
	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	memset(tmp[3], '\0', 2*nbytes);
	XP_mul(tmp[3], nbytes, x, nbytes, y);
	memcpy(z, tmp[3], (2*nbits-1)/8+1);

	return z;
}

T MP_mul2(T z, T x, T y)
{
	int sx;
	int sy;

	// check runtime errors for binary functions
	assert(x); assert(y); assert(z);

	// tmp[3] <-- x.y
	sx = sign(x);
	sy = sign(y);
	// if x < 0, negate x
	if(sx) {
		XP_neg(nbytes, tmp[0], x, 1);
		x = tmp[0];
		x[nbytes-1] &= msb;
	}

	// if y < 0, negate y
	if(sy) {
		XP_neg(nbytes, tmp[1], y, 1);
		y = tmp[1];
		y[nbytes-1] &= msb;
	}

	memset(tmp[3], '\0', 2*nbytes);
	XP_mul(tmp[3], nbytes, x, nbytes, y);

	if(sx != sy) {
		XP_neg((2*nbits-1)/8+1, z, tmp[3], 1);
	} else {
		memcpy(z, tmp[3], (2*nbits-1)/8+1);
	}
	return z;
}

T MP_addi(T z, T x, long y) // ERROR FIXME
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(-BASE<y && y<BASE) {
		int sx = sign(x);
		int sy = y < 0;
		if(sy) {
			XP_diff(nbytes, z, x, -y);
		} else {
			XP_sum(nbytes, z, x, y);
		}
		z[nbytes-1] &= msb;
		// test for signed overflow
		if(sx==sy && sy!=sign(z)){
			RAISE(MP_Overflow);
		}
		// check if signed y is too big
		if(nbits<8 && (y<-(1<<(nbits-1))||y>=(1<<(nbits-1)))) {
			RAISE(MP_Overflow);
		}
	} else if(apply(MP_add, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_subi(T z, T x, long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(-BASE<y && y<BASE) {
		int sx = sign(x);
		int sy = y < 0;
		if(sy) {
			XP_sum(nbytes, z, x, -y);
		} else {
			XP_diff(nbytes, z, x, y);
		}
		z[nbytes-1] &= msb;
		// test for signed underflow
		if(sx!=sy && sy==sign(z)){
			RAISE(MP_Overflow);
		}
		// check if signed y is too big
		if(nbits<8 && (y<-(1<<(nbits-1))||y>=(1<<(nbits-1)))) {
			RAISE(MP_Overflow);
		}
	} else if(apply(MP_sub, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_muli(T z, T x, long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(-BASE<y && y<BASE) {
		int sx = sign(x);
		int sy = y < 0;
		// if y < 0, negate y
		if(sy) {
			XP_neg(nbytes, tmp[1], y, 1);
			y = tmp[1];
			y[nbytes-1] &= msb;
		}
		
		XP_product(nbytes, z, x, sy? -y : y);
		if(sx != sy){
			XP_neg(nbytes, z, x, 1);
		}
		z[nbytes-1] &= msb;
		if(sx==sy && sign(z)) {
			RAISE(MP_Overflow);
		}
		// check if signed y is too big
		if(nbits<8 && (y<-(1<<(nbits-1))||y>=(1<<(nbits-1)))) {
			RAISE(MP_Overflow);
		}
	} else if(apply(MP_mul, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_divi(T z, T x, long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(y == 0) {
		RAISE(MP_Divdebyzero);
	} else if(-BASE<y && y<BASE) {
		int r;
		// z <- x/y, r <- x%y
		int sx = sign(x);
		int sy = y < 0;
		// if y < 0, negate y
		if(sy) {
			XP_neg(nbytes, tmp[1], y, 1);
			y = tmp[1];
			y[nbytes-1] &= msb;
		}
		r = XP_quotient(nbytes, z, x, sy? -y : y);
		if(sx != sy) {
			XP_neg(nbytes, z, z, 1);
			if(r != 0) {
				XP_diff(nbytes, z, z, 1);
				r = y - r;
			}
			z[nbytes-1] &= msb;
		} else if(sx && sign(z)) {
			RAISE(MP_Overflow);
		}

		// check if signed y is too big
		if(nbits<8 && (y<-(1<<(nbits-1))||y>=(1<<(nbits-1)))) {
			RAISE(MP_Overflow);
		}
	} else if(apply(MP_div, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_addui(T z, T x, unsigned long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(y < BASE){
		int carry = XP_sum(nbytes, z, x, y);
		carry |= z[nbytes-1] & ~msb;
		z[nbytes-1] &= msb;
		// test for unsigned overflow
		if(carry){
			RAISE(MP_Overflow);
		}
	} else (applyu(MP_addu, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_subui(T z, T x, unsigned long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(y < BASE) {
		int borrow = XP_diff(nbytes, z, x, y, z);
		borrow |= z[nbytes-1] & ~msb;
		z[nbytes-1] &= msb;
		// test for unsigned underflow
		if(borrow){
			RAISE(MP_Overflow);
		}
	} else if(applyu(MP_subu, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_mului(T z, T x, unsigned long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(y < BASE) {
		int carry = XP_product(nbytes, z, x, y);
		carry |= z[nbytes-1] & ~msb;
		z[nbytes-1] &= msb;
		// test for unsigned overflow
		if(carry){
			RAISE(MP_Overflow);
		}
		// check if unsigned y is too big
		if(nbits<8 && y>=(1U<<nbits)){
			RAISE(MP_Overflow);
		}
	} else if(applyu(MP_subu, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

T MP_divui(T z, T x, unsigned long y)
{
	// checked runtime error for unary functions
	assert(x);
	assert(z);

	if(y == 0){
		RAISE(MP_Divdebyzero);
	} else if(y < BASE){
		XP_quotient(nbytes, z, x, y);
		// check if unsigned y is too big
		if(nbits<8 && y>=(1U<<nbits)){
			RAISE(MP_Overflow);
		}
	} else if(applyu(MP_divu, z, x, y)) {
		RAISE(MP_Overflow);
	}

	return z;
}

long MP_modi(T x, long y)
{
	assert(x);

	if(y==0){
		RAISE(MP_Divdebyzero);
	} else if(-BASE<y && y<BASE) {
		T z = tmp[2];
		int r;
		// z <- x/y, r <- x%y
		int sx = sign(x);
		int sy = y < 0;
		// if y < 0, negate y
		if(sy) {
			XP_neg(nbytes, tmp[1], y, 1);
			y = tmp[1];
			y[nbytes-1] &= msb;
		}
		r = XP_quotient(nbytes, z, x, sy? -y : y);
		if(sx != sy) {
			XP_neg(nbytes, z, z, 1);
			if(r != 0) {
				XP_diff(nbytes, z, z, 1);
				r = y - r;
			}
			z[nbytes-1] &= msb;
		} else if(sx && sign(z)) {
			RAISE(MP_Overflow);
		}

		// check if signed y is too big
		if(nbits<8 && (y<-(1<<(nbits-1))||y>=(1<<(nbits-1)))) {
			RAISE(MP_Overflow);
		}
	} else if(apply(mp_mod, tmp[2], x, y)) {
		RAISE(MP_Overflow);
	}

	return MP_toint(tmp[2]);
}

unsigned long MP_modui(T x, unsigned long y)
{
	assert(x);

	if(y == 0) {
		RAISE(MP_Divdebyzero);
	} else if(y < BASE) {
		int r = XP_quotient(nbytes, tmp[2], x, y);
		// check if unsigned y is too big
		if(nbits<8 && y>=(1U<<nbits)){
			RAISE(MP_Overflow);
		}
		return r;
	} else if(applyu(MP_modu, tmp[2], x, y)) {
		RAISE(MP_Overflow);
	}

	return XP_toint(nbytes, tmp[2]);
}

int MP_cmp(T x, T y)
{
	int sx;
	int sy;

	assert(x);
	assert(y);

	sx = sign(x);
	sy = sign(y);

	if(sx != sy) {
		return sy - sx;
	} else {
		return XP_cmp(nbytes, x, y);
	}
}

int MP_cmpi(T x, long y)
{
	int sx;
	int sy = y < 0;

	assert(x);

	sx = sign(x);
	if(sx != sy) {
		return sy - sx;
	} else if((int)sizeof(y) >= nbytes) {
		long v = MP_toint(x);
		// return -1, 0, 1, if v<y, v=y, v>y
		return v<y? -1 : v>y? 1 : 0;
	} else {
		MP_fromint(tmp[2], y);
		return XP_cmp(nbytes, x, tmp[2]);
	}
}

int MP_cmpu(T x, T y)
{
	assert(x);
	assert(y);

	return XP_cmp(nbytes, x, y);
}

int MP_cmpui(T x, unsigned long y)
{
	assert(x);

	if((int)sizeof(y) >= nbytes) {
		unsigned long v = XP_toint(nbytes, x);
		// return -1, 0, 1, if v<y, v=y, v>y
		return v<y? -1 : v>y? 1 : 0;
	} else {
		XP_fromint(nbytes, tmp[2], y);
		return XP_cmp(nbytes, x, tmp[2]);
	}
}

T MP_and(T z, T x, T y)
{
	bitop(&);
}

T MP_or(T z, T x, T y)
{
	bitop(|);
}

T MP_xor(T z, T x, T y)
{
	bitop(^);
}

T MP_not(T z, T x)
{
	int i;

	// checked runtime error for unary functions
	assert(x);
	assert(z);

	for(i=0; i<nbytes; i++) {
		z[i] = ~x[i];
	}
	z[nbytes-1] &= msb;

	return z;
}

T MP_andi(T z, T x, unsigned long y)
{
	bitopi(MP_and);
}

T MP_ori(T z, T x, unsigned long y)
{
	bitopi(MP_or);
}

T MP_xori(T z, T x, unsigned y)
{
	bitopi(MP_xor);
}

T MP_lshift(T z, T x, int s)
{
	shft(0, XP_lshift);
}

T MP_rshift(T z, T x, int s)
{
	shft(0, XP_rshift);
}

T MP_ashift(T z, T x, int s)
{
	shft(sign(x), XP_rshift);
}

T MP_fromstr(T z, const char* str, int base, char** end)
{
	int carry;

	assert(z);

	memset(z, '\0', nbytes);
	carry = XP_fromstr(nbytes, z, str, base, end);
	carry |= z[nbytes-1] & ~msb;
	// test for unsigned overflow
	if(carry){
		RAISE(MP_Overflow);
	}

	return z;
}

char* MP_tostr(char* str, int size, int base, char** end)
{
	assert(x);
	assert(base>=2 && base<=36);
	assert(str==str || size>1);

	if(str == NULL) {
		// size <- number of characters to represent x int base
		int k;
		for(k=5; (1<<k)>base; k--){
			// nothing
		}
		size = nbits/k + 1 + 1;
		
		str = ALLOC(size);
	}
	memcpy(tmp[1], x, nbytes);
	XP_tostr(str, size, base, nbytes, tmp[1]);

	return str;
}

void MP_fmt(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	T x;
	int base;
	int size;
	int sx;
	char* buf;

	assert(app && flags);

	x = va_arg(*app, T);
	assert(x);
	base = va_arg(*app, int);
	assert(base>=2 && base<=36);
	sx = sign(x);
	// if x < 0, negate x
	if(sx) {
		XP_neg(nbytes, tmp[0], x, 1);
		x = tmp[0];
		x[nbytes-1] &= msb;
	}
	// size <- number of characters to represent x int base
	int k;
	for(k=5; (1<<k)>base; k--){
		// nothing
	}
	size = nbits/k + 1 + 1;

	buf = ALLOC(size + 1);
	if(sx) {
		buf[0] = '-';
		MP_tostr(buf+1, size, base, x);
	} else {
		MP_tostr(buf, size+1, base, x);
	}
	Fmt_putd(buf, strlen(buf), put, cl, flags, width, precision);
	FREE(buf);
	
}

void MP_fmtu(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	T x;
	char* buf;

	assert(app && flags);

	x = va_arg(*app, T);
	assert(x);

	buf = MP_tostr(NULL, 0, va_arg(*app, int), x);
	Fmt_putd(buf, strlen(buf), put, cl, flags, width, precision);
	FREE(buf);
}
