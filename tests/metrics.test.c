#include "metrics.h"
#include "process.h"
#include "unity.h"

void test_calculates_turnaround_from_finish_and_arrival(void) {
  Process process1 = {.arrival_time = 3, .finish_time = 13};
  int result = calculate_turnaround(&process1);

  TEST_ASSERT_EQUAL_INT(10, result);
}

void test_calculates_slowdown_using_total_cpu_time(void) {
  int cpu_vector[] = {2, 4};
  Process process1 = {.arrival_time = 2,
                      .finish_time = 14,
                      .cpu_bursts = cpu_vector,
                      .num_bursts = 2};

  double result = calculate_slowdown(&process1);

  TEST_ASSERT_DOUBLE_WITHIN(0.000001, 2.0, result);
}

void test_calculates_average_turnaround(void) {
  Process process1 = {.arrival_time = 1, .finish_time = 11};
  Process process2 = {.arrival_time = 3, .finish_time = 12};
  Process *processes[] = {&process1, &process2};

  double result = calculate_average_turnaround(processes, 2);

  TEST_ASSERT_DOUBLE_WITHIN(0.000001, 9.5, result);
}

void test_calculates_jain_index_over_process_slowdowns(void) {
  int cpu_vector1[] = {5};
  int cpu_vector2[] = {1, 2};
  Process process1 = {.arrival_time = 1,
                      .finish_time = 11,
                      .cpu_bursts = cpu_vector1,
                      .num_bursts = 1};
  Process process2 = {.arrival_time = 3,
                      .finish_time = 12,
                      .cpu_bursts = cpu_vector2,
                      .num_bursts = 2};
  Process *processes[] = {&process1, &process2};

  double result = calculate_jain_slowdown(processes, 2);

  TEST_ASSERT_DOUBLE_WITHIN(0.000001, 25.0 / 26.0, result);
}

void setUp(void) {}
void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_calculates_turnaround_from_finish_and_arrival);
  RUN_TEST(test_calculates_slowdown_using_total_cpu_time);
  RUN_TEST(test_calculates_average_turnaround);
  RUN_TEST(test_calculates_jain_index_over_process_slowdowns);
  return UNITY_END();
}
