CFLAGS := -Wall -Wextra -Wpedantic -std=c23

TEST_CASES := cases/valid cases/invalid cases/usage

run: test

test: testmain
	./testmain

valgrind: testmain
	valgrind ./testmain

cases/%: cases/%.c ../kwarg.h
	${CC} ${CFLAGS} -I.. $< -o $@

testmain: testmain.c unity.h unity_internals.h unity.c $(TEST_CASES) $(wildcard cases/*.expected.txt) $(wildcard cases/*.args.txt)
	${CC} ${CFLAGS} -g testmain.c unity.c -o $@

UPDATE_TARGETS := $(addprefix update-expected/,$(notdir $(TEST_CASES)))

update-expected: $(UPDATE_TARGETS)
	@echo "Updated expected output for all test cases"

update-expected/%: cases/%
	stem="$*"; \
	for af in cases/$$stem.*.args.txt; do \
		[ -f "$$af" ] || continue; \
		variant="$$(basename "$$af" .args.txt | sed 's/^'"$$stem"'\.//')"; \
		args="$$(cat "$$af")"; \
		"$<" $$args > "cases/$$stem.$${variant}.expected.txt" 2>&1 || true; \
	done
	@echo "Updated expected output for $*"

clean:
	rm -fv testmain $(TEST_CASES)

.PHONY: clean update-expected test valgrind run
