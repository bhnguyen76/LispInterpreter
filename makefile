CC = gcc

source = src/interpreter.c
header = src/interpreter.h

all: lisp run

lisp: src/main.c $(source) $(header)
	$(CC) -o lisp src/main.c $(source)

run: lisp
	./lisp

test: src/tests.c $(source) $(header)
	$(CC) -o test src/tests.c $(source)
	./test 

clean:
	rm -f run test *.o
