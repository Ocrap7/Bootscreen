CC=gcc
CFLAGS=-g
LD=gcc
LDFLAGS=-lpng -lm
SRC=$(wildcard **/*.c) $(wildcard *.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

all: main

all-clean: clean all

main: ${OBJ}
	${LD} ${LDFLAGS} $^ -o $@

%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@

clean:
	rm *.o