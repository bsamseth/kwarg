CFLAGS := -Wall -Wextra -Wpedantic -std=c23

TEST_CASES := tests/cases/basic tests/cases/required tests/cases/long_only

run: example
	./example --help

example: example.c argparse.h
	${CC} ${CFLAGS} -g $< -o $@

test: testmain
	./testmain

valgrind: testmain
	valgrind ./testmain

tests/cases/%: tests/cases/%.c argparse.h
	${CC} ${CFLAGS} -I. $< -o $@

testmain: testmain.c unity.h unity_internals.h unity.c $(TEST_CASES) $(wildcard tests/cases/*.expected.txt)
	${CC} ${CFLAGS} -g testmain.c unity.c -o $@

UPDATE_TARGETS := $(addprefix update-expected/,$(notdir $(TEST_CASES)))

update-expected: $(UPDATE_TARGETS)
	@echo "Updated expected output for all test cases"

update-expected/%: tests/cases/%
	"$<" --help 2>&1 > "$<.expected.txt"
	@echo "Updated $<.expected.txt"

clean:
	rm -fv testmain example $(TEST_CASES)

.PHONY: clean update-expected

