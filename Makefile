CFLAGS := -Wall -Wextra -Wpedantic -std=c23
MANPAGES := kwarg kwarg_init kwarg_str kwarg_flag kwarg_finish

run: example
	./example --help

example: example.c kwarg.h
	${CC} ${CFLAGS} -g $< -o $@

test:
	$(MAKE) -C tests test

valgrind:
	$(MAKE) -C tests valgrind

update-expected:
	$(MAKE) -C tests update-expected

man: $(addprefix docs/man/man3/, $(addsuffix .3, $(MANPAGES)))

docs/man/man3/%.3: docs/%.adoc
	@mkdir -p docs/man/man3
	asciidoctor -b manpage -o $@ $<

clean:
	$(MAKE) -C tests clean
	rm -fv example subcommand_example
	rm -fr docs/man

.PHONY: test valgrind update-expected clean run man
