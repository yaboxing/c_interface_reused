.SUFFIXES: .c .o
.DEFAULT: all

TARGET=libexcept.so
PLATFORM=
CC=gcc

ifeq (${CC},arm-linux-g++)
PLATFORM=-D__ARM__
endif

DEBUG=-D_DEBUG
DLD=
LIBS=
CFLAGS= -g -Wall -O0 -fPIC

INCLUDE=

SRC=../assert/assert.c \
	 ./except.c

OBJS=${SRC:.c=.o}

${TARGET}:${OBJS} ${EOBJS}
	${CC} ${PLATFORM} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS} -shared
	@echo ${TARGET} "build success"

.c.o:
# ${CC} -E -c $< -o $@.i
	${CC} ${PLATFORM} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
clean:
	@rm -f ${TARGET} ${OBJS}
	@echo ${OBJS} ${TARGET} " clean success"

