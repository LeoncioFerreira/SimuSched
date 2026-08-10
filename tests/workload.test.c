#include "workload.h"
#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>

void setUp(void) {
  // Inicializações antes de cada teste, se necessário
}

void tearDown(void) {
  // Limpezas após cada teste, se necessário
}

static ScenarioConfig get_test_config() {
  ScenarioConfig cfg = {.total_processes = 50,
                        .min_arrival = 0,
                        .max_arrival = 100,
                        .min_priority = 1,
                        .max_priority = 5,
                        .min_burst_duration = 10,
                        .max_burst_duration = 50,
                        .min_cpu_bursts = 2,
                        .max_cpu_bursts = 8};
  return cfg;
}

void test_reproducibility_same_seed(void) {
  ScenarioConfig cfg = get_test_config();
  Process *workload_a = generate_workload(cfg, 42);
  Process *workload_b = generate_workload(cfg, 42);

  for (int i = 0; i < cfg.total_processes; i++) {
    TEST_ASSERT_EQUAL_INT(workload_a[i].arrival_time,
                          workload_b[i].arrival_time);
    TEST_ASSERT_EQUAL_INT(workload_a[i].priority, workload_b[i].priority);
    TEST_ASSERT_EQUAL_INT(workload_a[i].num_bursts, workload_b[i].num_bursts);

    for (int j = 0; j < workload_a[i].num_bursts; j++) {
      TEST_ASSERT_EQUAL_INT(workload_a[i].bursts[j].type,
                            workload_b[i].bursts[j].type);
      TEST_ASSERT_EQUAL_INT(workload_a[i].bursts[j].duration,
                            workload_b[i].bursts[j].duration);
    }
  }

  free_workload(workload_a, cfg.total_processes);
  free_workload(workload_b, cfg.total_processes);
}

void test_variability_different_seeds(void) {
  ScenarioConfig cfg = get_test_config();
  Process *workload_a = generate_workload(cfg, 42);
  Process *workload_b = generate_workload(cfg, 99);

  bool is_different = false;
  for (int i = 0; i < cfg.total_processes; i++) {
    if (workload_a[i].arrival_time != workload_b[i].arrival_time ||
        workload_a[i].priority != workload_b[i].priority) {
      is_different = true;
      break;
    }
  }

  TEST_ASSERT_TRUE_MESSAGE(
      is_different, "Seeds diferentes devem produzir ao menos uma diferenca");

  free_workload(workload_a, cfg.total_processes);
  free_workload(workload_b, cfg.total_processes);
}

void test_burst_structure(void) {
  ScenarioConfig cfg = get_test_config();
  Process *workload = generate_workload(cfg, 123);

  for (int i = 0; i < cfg.total_processes; i++) {
    int last_burst_index = workload[i].num_bursts - 1;

    // Sempre comeca e termina em CPU
    TEST_ASSERT_EQUAL_INT_MESSAGE(BURST_CPU, workload[i].bursts[0].type,
                                  "Primeira rajada deve ser CPU");
    TEST_ASSERT_EQUAL_INT_MESSAGE(BURST_CPU,
                                  workload[i].bursts[last_burst_index].type,
                                  "Ultima rajada deve ser CPU");

    // Alternancia de estados
    for (int j = 0; j < last_burst_index; j++) {
      TEST_ASSERT_NOT_EQUAL_MESSAGE(workload[i].bursts[j].type,
                                    workload[i].bursts[j + 1].type,
                                    "As rajadas devem alternar");
    }
  }

  free_workload(workload, cfg.total_processes);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_reproducibility_same_seed);
  RUN_TEST(test_variability_different_seeds);
  RUN_TEST(test_burst_structure);
  return UNITY_END();
}