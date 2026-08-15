/**
 * Descrição: Valida a ordenação normal e o mecanismo de resgate da política
 * Triagem com Espera Justa (TEJ).
 * Autor: Leôncio Ferreira
 */
#include "tej.h"
#include "scheduler.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_rejects_invalid_parameters(void) {
  TEST_ASSERT_NULL(create_tej_scheduler(0, 0, 10));
  TEST_ASSERT_NULL(create_tej_scheduler(2, -1, 10));
  TEST_ASSERT_NULL(create_tej_scheduler(2, 0, 0));
  TEST_ASSERT_NULL(create_tej_scheduler(2, 0, -1));
}

void test_reports_empty_and_capacity(void) {
  Scheduler *scheduler = create_tej_scheduler(1, 0, 10);
  Process first = {.id = 1, .priority = 0, .ready_queue_arrival_time = 0};
  Process second = {.id = 2, .priority = 0, .ready_queue_arrival_time = 0};

  TEST_ASSERT_NOT_NULL(scheduler);
  TEST_ASSERT_TRUE(scheduler_is_empty(scheduler));
  TEST_ASSERT_NULL(scheduler_get_next_process(scheduler, 0));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &first));
  TEST_ASSERT_FALSE(scheduler_enqueue_process(scheduler, &second));
  TEST_ASSERT_FALSE(scheduler_is_empty(scheduler));
  TEST_ASSERT_EQUAL_PTR(&first, scheduler_get_next_process(scheduler, 0));
  TEST_ASSERT_TRUE(scheduler_is_empty(scheduler));
  scheduler_destroy(scheduler);
}

void test_normal_selection_uses_priority_arrival_and_id(void) {
  Scheduler *scheduler = create_tej_scheduler(4, 0, 100);
  Process low = {.id = 1, .priority = 4, .ready_queue_arrival_time = 0};
  Process later = {.id = 3, .priority = 1, .ready_queue_arrival_time = 2};
  Process greater_id = {.id = 5, .priority = 1, .ready_queue_arrival_time = 1};
  Process smaller_id = {.id = 2, .priority = 1, .ready_queue_arrival_time = 1};

  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &low));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &later));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &greater_id));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &smaller_id));

  TEST_ASSERT_EQUAL_PTR(&smaller_id, scheduler_get_next_process(scheduler, 10));
  TEST_ASSERT_EQUAL_PTR(&greater_id, scheduler_get_next_process(scheduler, 10));
  TEST_ASSERT_EQUAL_PTR(&later, scheduler_get_next_process(scheduler, 10));
  TEST_ASSERT_EQUAL_PTR(&low, scheduler_get_next_process(scheduler, 10));
  scheduler_destroy(scheduler);
}

void test_rescue_starts_exactly_at_deadline(void) {
  Scheduler *scheduler = create_tej_scheduler(2, 0, 10);
  Process waiting = {.id = 1, .priority = 2, .ready_queue_arrival_time = 0};
  Process urgent = {.id = 2, .priority = 0, .ready_queue_arrival_time = 29};

  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &waiting));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &urgent));
  TEST_ASSERT_EQUAL_PTR(&urgent, scheduler_get_next_process(scheduler, 29));
  scheduler_destroy(scheduler);

  scheduler = create_tej_scheduler(2, 0, 10);
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &waiting));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &urgent));
  TEST_ASSERT_EQUAL_PTR(&waiting, scheduler_get_next_process(scheduler, 30));
  scheduler_destroy(scheduler);
}

void test_rescued_process_precedes_new_maximum_priority(void) {
  Scheduler *scheduler = create_tej_scheduler(2, 0, 10);
  Process rescued = {.id = 1, .priority = 3, .ready_queue_arrival_time = 0};
  Process urgent = {.id = 2, .priority = 0, .ready_queue_arrival_time = 39};

  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &rescued));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &urgent));
  TEST_ASSERT_EQUAL_PTR(&rescued, scheduler_get_next_process(scheduler, 40));
  scheduler_destroy(scheduler);
}

void test_rescue_orders_by_deadline_then_arrival_then_id(void) {
  Scheduler *scheduler = create_tej_scheduler(4, 0, 10);
  Process later_deadline = {
      .id = 1, .priority = 1, .ready_queue_arrival_time = 0};
  Process earlier_deadline = {
      .id = 2, .priority = 0, .ready_queue_arrival_time = 5};
  Process greater_id = {.id = 5, .priority = 0, .ready_queue_arrival_time = 10};
  Process smaller_id = {.id = 3, .priority = 0, .ready_queue_arrival_time = 10};

  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &later_deadline));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &earlier_deadline));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &greater_id));
  TEST_ASSERT_TRUE(scheduler_enqueue_process(scheduler, &smaller_id));

  TEST_ASSERT_EQUAL_PTR(&earlier_deadline,
                        scheduler_get_next_process(scheduler, 25));
  TEST_ASSERT_EQUAL_PTR(&later_deadline,
                        scheduler_get_next_process(scheduler, 25));
  TEST_ASSERT_EQUAL_PTR(&smaller_id, scheduler_get_next_process(scheduler, 25));
  TEST_ASSERT_EQUAL_PTR(&greater_id, scheduler_get_next_process(scheduler, 25));
  scheduler_destroy(scheduler);
}

void test_rejects_process_priority_below_configured_minimum(void) {
  Scheduler *scheduler = create_tej_scheduler(1, 1, 10);
  Process invalid = {.id = 1, .priority = 0, .ready_queue_arrival_time = 0};

  TEST_ASSERT_FALSE(scheduler_enqueue_process(scheduler, &invalid));
  scheduler_destroy(scheduler);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_rejects_invalid_parameters);
  RUN_TEST(test_reports_empty_and_capacity);
  RUN_TEST(test_normal_selection_uses_priority_arrival_and_id);
  RUN_TEST(test_rescue_starts_exactly_at_deadline);
  RUN_TEST(test_rescued_process_precedes_new_maximum_priority);
  RUN_TEST(test_rescue_orders_by_deadline_then_arrival_then_id);
  RUN_TEST(test_rejects_process_priority_below_configured_minimum);
  return UNITY_END();
}
