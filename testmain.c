#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"

static void run_case(const char *name) {
  char cmd[512];
  char actual[16384];
  char expected[16384];
  char path[128];
  FILE *fp;

  // Check that the test binary was built by the Makefile
  snprintf(path, sizeof(path), "tests/cases/%s", name);
  fp = fopen(path, "r");
  if (!fp)
    TEST_FAIL_MESSAGE("Run 'make test' to build test binaries");
  fclose(fp);

  // Run with --help and capture output
  snprintf(cmd, sizeof(cmd), "tests/cases/%s --help 2>&1", name);
  fp = popen(cmd, "r");
  TEST_ASSERT_NOT_NULL_MESSAGE(fp, "popen failed");
  size_t n = fread(actual, 1, sizeof(actual) - 1, fp);
  actual[n] = '\0';
  pclose(fp);

  // Read expected output
  snprintf(path, sizeof(path), "tests/cases/%s.expected.txt", name);
  fp = fopen(path, "r");
  TEST_ASSERT_NOT_NULL_MESSAGE(fp, "Expected file not found");
  n = fread(expected, 1, sizeof(expected) - 1, fp);
  expected[n] = '\0';
  fclose(fp);

  TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, name);
}

static void test_basic(void) { run_case("basic"); }
static void test_required(void) { run_case("required"); }
static void test_long_only(void) { run_case("long_only"); }

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_basic);
  RUN_TEST(test_required);
  RUN_TEST(test_long_only);
  return UNITY_END();
}

void setUp(void) {}
void tearDown(void) {}
