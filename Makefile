CFLAGS := -Wall -Wextra -Wpedantic -std=c23

run: example
	./example --help

example: example.c argparse.h
	${CC} ${CFLAGS} -g $< -o $@

test: testmain
	./testmain

valgrind: testmain
	valgrind ./testmain

testmain: testmain.c unity.h unity_internals.h unity.c
	${CC} ${CFLAGS} -g testmain.c unity.c -o $@

clean:
	rm -fv testmain example

.PHONY: clean

