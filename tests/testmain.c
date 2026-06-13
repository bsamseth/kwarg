#define _GNU_SOURCE
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"

static int read_file(const char *path, char *buf, size_t bufsz) {
  FILE *fp = fopen(path, "r");
  if (!fp)
    return -1;
  size_t n = fread(buf, 1, bufsz - 1, fp);
  buf[n] = '\0';
  fclose(fp);
  return (int)n;
}

static void check_variant(const char *name, const char *variant,
                          const char *args) {
  char cmd[1024];
  char actual[8192];
  char expected[8192];
  char path[PATH_MAX];
  FILE *fp;
  size_t n;

  snprintf(cmd, sizeof(cmd), "cases/%s %s 2>&1", name, args);
  fp = popen(cmd, "r");
  TEST_ASSERT_NOT_NULL_MESSAGE(fp, "popen failed");
  n = fread(actual, 1, sizeof(actual) - 1, fp);
  actual[n] = '\0';
  pclose(fp);

  snprintf(path, sizeof(path), "cases/%s.%s.expected.txt", name, variant);

  if (read_file(path, expected, sizeof(expected)) < 0) {
    snprintf(actual, sizeof(actual),
             "Missing expected file for '%s' — run 'make update-expected/%s'",
             path, name);
    TEST_FAIL_MESSAGE(actual);
    return;
  }

  snprintf(path, sizeof(path), "%s/%s", name, variant);
  TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, path);
}

static void run_case(const char *name) {
  char path[PATH_MAX];

  // Check binary exists
  snprintf(path, sizeof(path), "cases/%s", name);
  FILE *fp = fopen(path, "r");
  if (!fp)
    TEST_FAIL_MESSAGE("Run 'make test' to build test binaries");
  fclose(fp);

  // Discover variants by scanning for <name>.<variant>.args.txt files
  char prefix[1024];
  snprintf(prefix, sizeof(prefix), "%s.", name);
  DIR *dir = opendir("cases");
  if (!dir)
    return;

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strncmp(entry->d_name, prefix, strlen(prefix)) != 0)
      continue;
    const char *rest = entry->d_name + strlen(prefix);
    size_t rest_len = strlen(rest);
    if (rest_len < 9 || strcmp(rest + rest_len - 9, ".args.txt") != 0)
      continue;

    // Extract variant name: strip trailing ".args.txt"
    char variant[1024];
    size_t vlen = rest_len - 9;
    memcpy(variant, rest, vlen);
    variant[vlen] = '\0';

    // Read args from file (trim trailing newline)
    char args[1024] = {0};
    snprintf(path, sizeof(path), "cases/%s", entry->d_name);
    int n = read_file(path, args, sizeof(args));
    if (n < 0)
      continue;
    while (n > 0 && (args[n - 1] == '\n' || args[n - 1] == '\r'))
      args[--n] = '\0';

    check_variant(name, variant, args);
  }
  closedir(dir);
}

static void test_valid(void) { run_case("valid"); }
static void test_invalid(void) { run_case("invalid"); }
static void test_usage(void) { run_case("usage"); }

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_valid);
  RUN_TEST(test_invalid);
  RUN_TEST(test_usage);
  return UNITY_END();
}

void setUp(void) {}
void tearDown(void) {}
