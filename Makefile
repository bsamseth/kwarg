CFLAGS := -Wall -Wextra -Wpedantic -std=c23

run: example
	./example --help

example: example.c argparse.h
	${CC} ${CFLAGS} -g $< -o $@

test:
	$(MAKE) -C tests test

valgrind:
	$(MAKE) -C tests valgrind

update-expected:
	$(MAKE) -C tests update-expected

clean:
	$(MAKE) -C tests clean
	rm -fv example subcommand_example

.PHONY: test valgrind update-expected clean run
