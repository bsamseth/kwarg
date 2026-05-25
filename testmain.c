#include <assert.h>

#include "unity.h"

void test_placeholder(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_placeholder);
  return UNITY_END();
}
void setUp(void) {}
void tearDown(void) {}
