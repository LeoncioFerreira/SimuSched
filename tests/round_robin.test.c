/**
 * Descrição: Testes unitários da ordenação FIFO usada pelo escalonador Round
 * Robin por meio da interface comum de políticas.
 * Autor: Leôncio Ferreira
 */
#include "round_robin.h"
#include "scheduler.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_round_robin_selects_processes_in_fifo_order(void) {
  Process first = {.id = 1};
  Process second = {.id = 2};
  Scheduler *scheduler = create_round_robin_scheduler(2);

  TEST_ASSERT_NOT_NULL(scheduler);
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &first));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &second));
  TEST_ASSERT_EQUAL_PTR(&first, scheduler_get_next_process(scheduler));
  TEST_ASSERT_EQUAL_PTR(&second, scheduler_get_next_process(scheduler));
  TEST_ASSERT_TRUE(scheduler_is_empty(scheduler));

  scheduler_destroy(scheduler);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_round_robin_selects_processes_in_fifo_order);
  return UNITY_END();
}
