.SUFFIXES: .c .o
.DEFAULT: all

TARGET=libap.so
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
    ../except/except.c \
    ../format/format.c \
    ../mem/mem.c \
    ../xp/xp.c \
    ./ap.c

OBJS=${SRC:.c=.o}

${TARGET}:${OBJS}
	${CC} ${PLATFORM} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS} -shared
	@echo ${TARGET} "build success"

.c.o:
#	${CC} -E -c $< -o $@.i
	${CC} ${PLATFORM} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
clean:
	@rm -f ${TARGET} ${OBJS}
	@echo ${OBJS} ${TARGET} " clean success"
