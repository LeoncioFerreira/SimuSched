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

void test_balanced_stats(void) {
  Scenario scenario;
  char error[256];
  ConfigParseResult res = config_parse_file("configs/balanced.conf", &scenario,
                                            error, sizeof(error));
  TEST_ASSERT_EQUAL(CONFIG_PARSE_OK, res);
  TEST_ASSERT_EQUAL(1000, scenario.config.total_processes);

  Process **workload = generate_workload(&scenario.config, 555);
  TEST_ASSERT_NOT_NULL(workload);

  double avg_cpu, avg_io, avg_bursts, high_prio;
  calculate_stats(workload, scenario.config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);

  // CPU and IO have intermediate values (5-50)
  TEST_ASSERT_TRUE(avg_cpu > 15.0 && avg_cpu < 40.0);
  TEST_ASSERT_TRUE(avg_io > 15.0 && avg_io < 40.0);

  // High priority ratio should be around 50%
  TEST_ASSERT_TRUE(high_prio >= 0.40 && high_prio <= 0.60);

  free_workload(workload, scenario.config.total_processes);
}

void test_ratio_limits(void) {
  ScenarioConfig config = {
      .total_processes = 100,
      .min_arrival = 0,
      .max_arrival = 1000,
      .min_priority = 0,
      .max_priority = 1,
      .high_priority_ratio = 0.0,
      .min_cpu_burst_duration = 10,
      .max_cpu_burst_duration = 20,
      .min_io_burst_duration = 10,
      .max_io_burst_duration = 20,
      .min_cpu_bursts = 2,
      .max_cpu_bursts = 5,
      .context_switch_cost = 1,
  };

  Process **workload = generate_workload(&config, 111);
  TEST_ASSERT_NOT_NULL(workload);

  double avg_cpu, avg_io, avg_bursts, high_prio;
  calculate_stats(workload, config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);
  TEST_ASSERT_TRUE(high_prio == 0.0);
  free_workload(workload, config.total_processes);

  config.high_priority_ratio = 1.0;
  workload = generate_workload(&config, 222);
  TEST_ASSERT_NOT_NULL(workload);
  calculate_stats(workload, config.total_processes, &avg_cpu, &avg_io,
                  &avg_bursts, &high_prio);
  TEST_ASSERT_TRUE(high_prio == 1.0);
  free_workload(workload, config.total_processes);
}

void test_nan_rejection(void) {
  FILE *f = fopen("configs/test_nan.conf", "w");
  fprintf(f, "scenario=test\ntotal_processes=10\nmin_arrival=0\nmax_arrival=1\n"
             "min_priority=0\nmax_priority=1\nhigh_priority_ratio=NaN\n"
             "min_cpu_burst_duration=1\nmax_cpu_burst_duration=1\n"
             "min_io_burst_duration=1\nmax_io_burst_duration=1\n"
             "min_cpu_bursts=1\nmax_cpu_bursts=1\nquantum=1\n"
             "context_switch_cost=1\n");
  fclose(f);

  Scenario scenario;
  char error[256];
  ConfigParseResult res = config_parse_file("configs/test_nan.conf", &scenario,
                                            error, sizeof(error));
  TEST_ASSERT_EQUAL(CONFIG_PARSE_ERROR, res);
  remove("configs/test_nan.conf");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_io_bound_stats);
  RUN_TEST(test_cpu_bound_stats);
  RUN_TEST(test_unbalanced_stats);
  RUN_TEST(test_balanced_stats);
  RUN_TEST(test_ratio_limits);
  RUN_TEST(test_nan_rejection);
  return UNITY_END();
}
