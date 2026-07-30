/**
 * Descrição: Exemplo temporário criado apenas para o pipeline executar build e
 * lint.
 */

#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_example_assertion(void) { TEST_ASSERT_EQUAL_INT(1, 1); }

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_example_assertion);
  return UNITY_END();
}
