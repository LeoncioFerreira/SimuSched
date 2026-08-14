#include "workload.h"
#include "unity.h"
#include <stdbool.h>

void setUp(void) {}

void tearDown(void) {}

static ScenarioConfig get_test_config(void) {
  ScenarioConfig config = {
      .total_processes = 50,
      .min_arrival = 0,
      .max_arrival = 100,
      .min_priority = 1,
      .max_priority = 5,
      .high_priority_ratio = 0.5,
      .min_cpu_burst_duration = 10,
      .max_cpu_burst_duration = 50,
      .min_io_burst_duration = 10,
      .max_io_burst_duration = 50,
      .min_cpu_bursts = 2,
      .max_cpu_bursts = 8,
  };
  return config;
}

static void assert_processes_equal(const Process *first,
                                   const Process *second) {
  TEST_ASSERT_EQUAL_INT(first->id, second->id);
  TEST_ASSERT_EQUAL_INT(first->arrival_time, second->arrival_time);
  TEST_ASSERT_EQUAL_INT(first->priority, second->priority);
  TEST_ASSERT_EQUAL_INT(first->state, second->state);
  TEST_ASSERT_EQUAL_INT(first->num_bursts, second->num_bursts);

  for (int j = 0; j < first->num_bursts; j++) {
    TEST_ASSERT_EQUAL_INT(first->cpu_bursts[j], second->cpu_bursts[j]);
    if (j < first->num_bursts - 1)
      TEST_ASSERT_EQUAL_INT(first->io_bursts[j], second->io_bursts[j]);
  }
}

static bool processes_differ(const Process *first, const Process *second) {
  if (first->arrival_time != second->arrival_time ||
      first->priority != second->priority ||
      first->num_bursts != second->num_bursts)
    return true;

  for (int j = 0; j < first->num_bursts; j++) {
    if (first->cpu_bursts[j] != second->cpu_bursts[j] ||
        (j < first->num_bursts - 1 &&
         first->io_bursts[j] != second->io_bursts[j]))
      return true;
  }

  return false;
}

void test_reproducibility_same_seed(void) {
  ScenarioConfig config = get_test_config();
  Process **workload_a = generate_workload(&config, 42);
  Process **workload_b = generate_workload(&config, 42);

  TEST_ASSERT_NOT_NULL(workload_a);
  TEST_ASSERT_NOT_NULL(workload_b);

  for (int i = 0; i < config.total_processes; i++)
    assert_processes_equal(workload_a[i], workload_b[i]);

  free_workload(workload_a, config.total_processes);
  free_workload(workload_b, config.total_processes);
}

void test_variability_different_seeds(void) {
  ScenarioConfig config = get_test_config();
  Process **workload_a = generate_workload(&config, 42);
  Process **workload_b = generate_workload(&config, 99);
  bool is_different = false;

  TEST_ASSERT_NOT_NULL(workload_a);
  TEST_ASSERT_NOT_NULL(workload_b);

  for (int i = 0; i < config.total_processes && !is_different; i++)
    is_different = processes_differ(workload_a[i], workload_b[i]);

  TEST_ASSERT_TRUE(is_different);
  free_workload(workload_a, config.total_processes);
  free_workload(workload_b, config.total_processes);
}

void test_distinct_32_bit_seeds_do_not_collide(void) {
  ScenarioConfig config = get_test_config();
  Process **workload_a = generate_workload(&config, 0U);
  Process **workload_b = generate_workload(&config, 0x80000000U);
  bool is_different = false;

  TEST_ASSERT_NOT_NULL(workload_a);
  TEST_ASSERT_NOT_NULL(workload_b);

  for (int i = 0; i < config.total_processes && !is_different; i++)
    is_different = processes_differ(workload_a[i], workload_b[i]);

  TEST_ASSERT_TRUE(is_different);
  free_workload(workload_a, config.total_processes);
  free_workload(workload_b, config.total_processes);
}

void test_generated_process_structure(void) {
  ScenarioConfig config = get_test_config();
  Process **workload = generate_workload(&config, 123);

  TEST_ASSERT_NOT_NULL(workload);

  for (int i = 0; i < config.total_processes; i++) {
    Process *process = workload[i];
    TEST_ASSERT_EQUAL_INT(i, process->id);
    TEST_ASSERT_EQUAL_INT(STATE_NEW, process->state);
    TEST_ASSERT_GREATER_OR_EQUAL(config.min_cpu_bursts, process->num_bursts);
    TEST_ASSERT_LESS_OR_EQUAL(config.max_cpu_bursts, process->num_bursts);
    TEST_ASSERT_NOT_NULL(process->cpu_bursts);
    if (process->num_bursts > 1) {
      TEST_ASSERT_NOT_NULL(process->io_bursts);
    }

    for (int j = 0; j < process->num_bursts; j++) {
      TEST_ASSERT_INT_WITHIN(20, 30, process->cpu_bursts[j]);
      if (j < process->num_bursts - 1)
        TEST_ASSERT_INT_WITHIN(20, 30, process->io_bursts[j]);
    }
  }

  free_workload(workload, config.total_processes);
}

void test_invalid_config_is_rejected(void) {
  ScenarioConfig config = get_test_config();

  TEST_ASSERT_NULL(generate_workload(NULL, 42));

  config.total_processes = 0;
  TEST_ASSERT_NULL(generate_workload(&config, 42));

  config = get_test_config();
  config.min_arrival = config.max_arrival + 1;
  TEST_ASSERT_NULL(generate_workload(&config, 42));

  config = get_test_config();
  config.min_cpu_burst_duration = 0;
  TEST_ASSERT_NULL(generate_workload(&config, 42));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_reproducibility_same_seed);
  RUN_TEST(test_variability_different_seeds);
  RUN_TEST(test_distinct_32_bit_seeds_do_not_collide);
  RUN_TEST(test_generated_process_structure);
  RUN_TEST(test_invalid_config_is_rejected);
  return UNITY_END();
}
