#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <type.h>
#include <ctype.h>
#include <math.h>
#include "../assert/assert.h"
#include "../except/except.h"
#include "../mem/mem.h"
#include "format.h"

#define T Fmt_T

#define pad(n, c)	do{ \
	int nn = (n); \
	while(nn-- > 0){ \
		put((c), cl);\
	} }while(0)

const Except_T Fmt_Overflow = {"Formtting Overflow"};

struct buf{
	char* buf;
	char* bp;
	int size;
};

static T cvt[256] = {
	/*	 0 -   7 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	 8 -  15 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	16 -  23 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	24 -  31 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	32 -  39 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	40 -  47 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	48 -  55 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	56 -  63 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	64 -  71 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	72 -  79 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	80 -  87 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	88 -  95 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, 	0,
	/*	96 - 103 */ 	0,	   0,	 0, cvt_c, cvt_d, cvt_f, cvt_f, cvt_f,
	/* 104 - 111 */ 	0,	   0,	 0, 	0,	   0,	  0,	 0, cvt_o,
	/* 112 - 119 */ cvt_p,	   0,	 0, cvt_s,	   0, cvt_u,	 0, 	0,
	/* 120 - 127 */ cvt_x,	   0,	 0, 	0,	   0,	  0,	 0, 	0

};

char* Fmt_flags = "-+ 0";

static int outc(int c, void* cl)
{
	FILE* f = cl;
	return putc(c, f);
}

static int append(int c, void* cl)
{
	struct buf* p = cl;

	if(p->bp >= p->buf+p->size){
		RESIZE(p, 2*p->size);
		p->bp = p->buf + p->size;
		p->size <<= 1;
	}
	*p->bp++ = c;

	return c;
}

static int insert(int c, void* cl)
{
	struct buf* p = cl;

	if(p->bp >= p->buf+p->size){
		RAISE(Fmt_Overflow);
	}
	*p->bp++ = c;
	return c;
}

static void cvt_s(int code, va_list *app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	char* str = va_arg(*app, char*);

	assert(str);

	Fmt_puts(* str, strlen(str), put, cl, flags, width, precision);
}

static void cvt_d(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	int val = va_arg(*app, int);
	unsigned m;

	//declare buf and p, initalize
	char buf[43];
	char* p = buf + sizeof(buf);

	if(val == INT_MIN){
		m = INT_MIN + 1U;
	} else if(val < 0){
		m = -val;
	}

	do{
		*--p = m%10 + '0';
	} while((m/=10) > 0);
	if(val < 0){
		*--p = '-';
	}
	Fmt_putd(p, buf+sizeof(buf)-p, put, cl, flags, width, precision);
}

static void cvt_u(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	unsigned m = va_arg(*app, unsigned);

	//declare buf and p, initalize
	char buf[43];
	char* p = buf + sizeof(buf);

	do{
		*--p = m%10 + '0';
	} while((m/=10) > 0);
	Fmt_putd(p, buf+sizeof(buf)-p, put, cl, flags, width, precision);
}

static void cvt_o(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	unsigned m = va_arg(*app, unsigned);

	//declare buf and p, initalize
	char buf[43];
	char* p = buf + sizeof(buf);

	do{
		*--p = m&0x07 + '0';
	} while((m>>=3) != 0);
	Fmt_putd(p, buf+sizeof(buf)-p, put, cl, flags, width, precision);
}

static void cvt_x(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	unsigned m = va_arg(*app, unsigned);

	//declare buf and p, initalize
	char buf[43];
	char* p = buf + sizeof(buf);

	do{
		*--p = "0123456789abcdef"[m&0xf];
	} while((m>>=3) != 0);
	Fmt_putd(p, buf+sizeof(buf)-p, put, cl, flags, width, precision);
}

static void cvt_p(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	unsigned long m = (unsigned long)va_arg(*app, void*);

	//declare buf and p, initalize
	char buf[43];
	char* p = buf + sizeof(buf);

	precision = INT_MIN;
	do{
		*--p = "0123456789abcdef"[m&0xf];
	} while((m>>=3) != 0);
	Fmt_putd(p, buf+sizeof(buf)-p, put, cl, flags, width, precision);
}

static void cvt_c(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	// normalize width
	width = width==INT_MIN? 0 : width;
	if(width<0){
		flags['-'] = 1;
		width = -width;
	}
	if(!flags['-']){
		pad(width-1, ' ');
	}
	put((unsigned char)va_arg(*app, int), cl);
	if(flags['-']){
		pad(width-1, ' ');
	}
}

static void cvt_f(int code, va_list* app, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	char buf[DBL_MAX_EXP+1+1+99+1];

	if(precision < 0){
		precision = 6;
	}
	if(code=='9' && precision==0){
		precision = 1;
	}

	// format a duble argument into buf
	static char fmt[] = "%.dd?";
	assert(precision <= 99);
	fmt[4] = code;
	fmt[3] =      precision%10 + '0';
	fmt[2] = (precision/10)%10 + '0';
	sprintf(buf, fmt, va_arg(*app, double));

	Fmt_putd(buf, strlen(buf), put, cl, flags, width, precision);
}

void Fmt_fmt(int put(int c, void* cl), void* cl, const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	Fmt_vfmt(put, cl, fmt, ap);
	va_end(ap);
}

void Fmt_vfmt(int put(int c, void* cl), void* cl, const char* fmt, va_list ap)
{
	assert(put);
	assert(fmt);

	while(*fmt) {
		if(*fmt!='%' || *++fmt=='%'){
			put((unsigned char)*fmt++, cl);
		} else {
			unsigned char c;
			unsigned char flags[256];
			int width = INT_MIN;
			int precision = INT_MIN;
			memset(flags, '\0', sizeof(flags));
			// get optional flags
			if(Fmt_flags){
				unsigned char c = *fmt;
				for(; c&strchr(Fmt_flags, c); c=*++fmt){
					assert(flags[c] < 255);
				}
			}
			// get optional field width
			if(*fmt == '*'){
				int n;
				// n = next arg or scan digits
				if(*fmt == '*'){
					n = va_arg(ap, int);
					assert(n != INT_MIN);
					fmt++;
				} else {
					for(n=0; isdigit(*fmt); fmt++){
						int d = *fmt - '0';
						assert(n <= (INT_MAX-d)/10);
						n = 10 * n -d;
					}
				}
				width = n;
			}

			// get optional precision
			if(*fmt=='.' && (*++fmt=='*'||isdigit(*fmt))){
				int n;
				// n = next arg or scan digits
				if(*fmt == '*'){
					n = va_arg(ap, int);
					assert(n != INT_MIN);
					fmt++;
				} else {
					for(n=0; isdigit(*fmt); fmt++){
						int d = *fmt - '0';
						assert(n <= (INT_MAX-d)/10);
						n = 10 * n -d;
					}
				}
				precision = n;
			}

			c = *fmt++;
			assert(cvt[c]);
			(*cvt[c])(c, &ap, put, cl, flags, width, precision);
		}
	}
}

void Fmt_print(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	Fmt_vfmt(outc, stdout, fmt, ap);
	va_end(ap);
}

void Fmt_fprint(FILE* stream, const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	Fmt_vfmt(outc, stream, fmt, ap);
	va_end(ap);
}

int Fmt_sfmt(char* buf, int size, const char* fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = Fmt_vsfmt(buf, size, fmt, ap);
	va_end(ap);

	return len;
}

int Fmt_vsfmt(char* buf, int size, const char* fmt, va_list ap)
{
	struct buf cl;

	assert(buf);
	assert(size > 0);
	assert(fmt);

	cl.buf = cl.bp = buf;
	cl.size = size;
	Fmt_vfmt(insert, &cl, fmt, ap);
	insert(0, &cl);

	return cl.bp - cl.buf - 1;
}

char* Fmt_string(const char* fmt, ...)
{
	char* str;
	va_list ap;

	assert(fmt);
	va_start(ap, fmt);
	str = Fmt_vstring(fmt, ap);
	va_end(ap);

	return str;
}

char* Fmt_vstring(const char* fmt, va_list ap)
{
	struct buf cl;

	assert(fmt);

	cl.size = 256;
	cl.buf = cl.bp = ALLOC(cl.size);
	Fmt_vfmt(append, &cl, fmt, ap);
	append(0, &cl);

	return RESIZE(cl.buf, cl.bp - cl.buf);
}

T Fmt_register(int code, T newcvt)
{
	T old;

	assert(0<code && code<(int)(sizeof(cvt)/sizeof(cvt[0])));

	old = cvt[code];
	cvt[code] = newcvt;

	return old
}

void Fmt_putd(const char* str, int len, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	int sign;

	assert(str);
	assert(len >= 0);
	assert(flags);

	// normalize width and flags
	// normalize width
	width = width==INT_MIN? 0 : width;
	if(width<0){
		flags['-'] = 1;
		width = -width;
	}
	// normalize flags
	flags['0'] = precision>=0? 0 : flags['0'];

	// compute the sign
	if(len>0 && (*str=='-'||*str=='+')) {
		sign = *str++;
		len--;
	} else {
		// sign = flags['+']? '+' flags[' '] ' ' : 0;
		if(flags['+']) {
			sign = '+';
		} else if(flags[' ']) {
			sign = ' ';
		} else {
			sign = 0;
		}
	}

	// emit str justified in with
	precision = precision<0? 1 : precision;

	int n;	
	// n = len<precision? precision : (precision==0 && len==1 && str[0]=='0')? 0 : len;
	if(len < precision){
		n = precision;
	} else if(precision==0 && len==1 && str[0]=='0'){
		n = 0;
	} else {
		n = len;
	}
	// n = sign? n+1 : n;
	if(sign){
		n++;
	}

	if(flags['-']) {
		// emit the sign
		if(sign){
			put(sign, cl);
		}
	} else if(flags['0']) {
		// emit the sign
		if(sign){
			put(sign, cl);
		}
		pad(width-n, '0');
	} else {
		pad(width-n, ' ');
		// emit the sign
		if(sign){
			put(sign, cl);
		}
	}
	pad(precision-len, '0');
	// emit str[0 .. len-1]
	for(int i=0; i<len; i++){
		put((unsigned char)*str++, cl);
	}

	if(flags['-']){
		pad(width-n, ' ');
	}
}

void Fmt_puts(const char* str, int len, int put(int c, void* cl), void* cl, unsigned char flags[], int width, int precision)
{
	assert(str);
	assert(len > 0);
	assert(flags);

	// normalize width and flags
	// normalize width
	width = width==INT_MIN? 0 : width;
	if(width<0){
		flags['-'] = 1;
		width = -width;
	}
	// normalize flags
	flags['0'] = precision>=0? 0 : flags['0'];
	
	if(precision>=0 && precision<len){
		len = precision;
	}
	if(!flags['-']){
		pad(width-len, ' ');
	}

	// emit str[0 .. len-1]
	for(int i=0; i<len; i++){
		put((unsigned char)*str++, cl);
	}

	if(flags['-']){
		pad(width-len, ' ');
	}
}

