#include "config_parser.h"
#include "unity.h"
#include "workload.h"
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

static void calculate_stats(Process **workload, int num_processes,
                            double *avg_cpu_burst, double *avg_io_burst,
                            double *avg_num_bursts,
                            double *high_priority_ratio) {
  long long total_cpu_burst_time = 0;
  long long total_io_burst_time = 0;
  long long total_cpu_bursts = 0;
  long long total_io_bursts = 0;
  int high_priority_count = 0;

  for (int i = 0; i < num_processes; i++) {
    Process *p = workload[i];
    total_cpu_bursts += p->num_bursts;
    if (p->num_bursts > 1) {
      total_io_bursts += (p->num_bursts - 1);
    }

    for (int j = 0; j < p->num_bursts; j++) {
      total_cpu_burst_time += p->cpu_bursts[j];
      if (j < p->num_bursts - 1) {
        total_io_burst_time += p->io_bursts[j];
      }
    }

    // Min priority in our configs is 0.
    // Generator gives min_priority if it falls into high_priority_ratio
    if (p->priority == 0) {
      high_priority_count++;
    }
  }

  *avg_cpu_burst = total_cpu_bursts > 0
                       ? (double)total_cpu_burst_time / total_cpu_bursts
                       : 0;
  *avg_io_burst =
      total_io_bursts > 0 ? (double)total_io_burst_time / total_io_bursts : 0;
  *avg_num_bursts = (double)total_cpu_bursts / num_processes;
  *high_priority_ratio = (double)high_priority_count / num_processes;
}

void test_io_bound_stats(void) {
  Scenario scenario;
  char error[256];
  ConfigParseResult res = config_parse_file("configs/io_bound.conf", &scenario,
                                            error, sizeof(error));
  TEST_ASSERT_EQUAL(CONFIG_PARSE_OK, res);
  TEST_ASSERT_EQUAL(1000, scenario.config.total_processes);

  Process **workload = generate_workload(&scenario.config, 42);
  TEST_ASSERT_NOT_NULL(workload);

  double avg_cpu, avg_io, avg_bursts, high_prio;
  calculate_stats(workload, scenario.config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);

  // Config: CPU 1-5, IO 30-100, Bursts 10-30
  TEST_ASSERT_TRUE(avg_cpu < 6.0);
  TEST_ASSERT_TRUE(avg_io >= 25.0);
  TEST_ASSERT_TRUE(avg_bursts >= 9.0);

  free_workload(workload, scenario.config.total_processes);
}

void test_cpu_bound_stats(void) {
  Scenario scenario;
  char error[256];
  ConfigParseResult res = config_parse_file("configs/cpu_bound.conf", &scenario,
                                            error, sizeof(error));
  TEST_ASSERT_EQUAL(CONFIG_PARSE_OK, res);

  Process **workload = generate_workload(&scenario.config, 84);
  TEST_ASSERT_NOT_NULL(workload);

  double avg_cpu, avg_io, avg_bursts, high_prio;
  calculate_stats(workload, scenario.config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);

  // Config: CPU 50-200, IO 1-5, Bursts 1-3
  TEST_ASSERT_TRUE(avg_cpu >= 45.0);
  TEST_ASSERT_TRUE(avg_io < 6.0);
  TEST_ASSERT_TRUE(avg_bursts <= 4.0);

  free_workload(workload, scenario.config.total_processes);
}

void test_unbalanced_stats(void) {
  Scenario scenario;
  char error[256];
  ConfigParseResult res = config_parse_file(
      "configs/unbalanced_priorities.conf", &scenario, error, sizeof(error));
  TEST_ASSERT_EQUAL(CONFIG_PARSE_OK, res);

  Process **workload = generate_workload(&scenario.config, 12345);
  TEST_ASSERT_NOT_NULL(workload);

  double avg_cpu, avg_io, avg_bursts, high_prio;
  calculate_stats(workload, scenario.config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);

  // Configured ratio is 0.85 (85%)
  TEST_ASSERT_TRUE(high_prio >= 0.75 && high_prio <= 0.95);

  free_workload(workload, scenario.config.total_processes);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_io_bound_stats);
  RUN_TEST(test_cpu_bound_stats);
  RUN_TEST(test_unbalanced_stats);
  return UNITY_END();
}
