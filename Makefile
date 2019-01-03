.SUFFIXES: .c .o
.DEFAULT: all

TARGET=libtools.so
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

SRC=./assert/assert.c \
    ./except/except.c \
    ./arith/arith.c \
    ./stack/stack.c \
    ./atom/atom.c \
    ./arena/arena.c \
    ./list/list.c \
    ./table/table.c \
    ./set/set.c \
    ./array/array.c \
    ./ring/ring.c \
    ./bit/bit.c \
    ./str/str.c \
    ./text/text.c \
    ./format/format.c \
    ./mem/mem.c \
    ./ap/ap.c \
    ./xp/xp.c \
    ./mp/mp.c

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
