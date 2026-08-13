#include "cli.h"
#include "simulator.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>

static const char *fcfs_path = "tests/tmp_fcfs.csv";
static const char *priority_path = "tests/tmp_priority.csv";
static const char *round_robin_path = "tests/tmp_round_robin.csv";
static const char *switch_config_path = "tests/tmp_context_switch.conf";

void setUp(void) {
  remove(fcfs_path);
  remove(priority_path);
  remove(round_robin_path);
  remove(switch_config_path);
}

void tearDown(void) {
  remove(fcfs_path);
  remove(priority_path);
  remove(round_robin_path);
  remove(switch_config_path);
}

static void write_switch_config(void) {
  FILE *file = fopen(switch_config_path, "w");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_TRUE(fputs("scenario=context-switch\n"
                         "total_processes=2\n"
                         "min_arrival=0\nmax_arrival=0\n"
                         "min_priority=1\nmax_priority=1\n"
                         "min_burst_duration=1\nmax_burst_duration=1\n"
                         "min_cpu_bursts=1\nmax_cpu_bursts=1\n"
                         "quantum=1\ncontext_switch_cost=10\n",
                         file) >= 0);
  TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

static void assert_run_writes_csv(const char *algorithm, const char *path) {
  CliOptions options = {algorithm, 42U, "configs/small.conf", path};
  char error[256];
  char header[256];
  char row[256];
  FILE *file;

  TEST_ASSERT_TRUE(run_simulator(&options, error, sizeof(error)));
  file = fopen(path, "r");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_NOT_NULL(fgets(header, sizeof(header), file));
  TEST_ASSERT_EQUAL_STRING(
      "algorithm,seed,scenario,total_processes,total_simulated_time\n", header);
  TEST_ASSERT_NOT_NULL(fgets(row, sizeof(row), file));
  fclose(file);
  TEST_ASSERT_EQUAL_INT(0, strncmp(row, algorithm, strlen(algorithm)));
  TEST_ASSERT_NOT_NULL(strstr(row, ",42,small,5,"));
}

void test_runs_small_scenario_with_fcfs(void) {
  assert_run_writes_csv("fcfs", fcfs_path);
}

void test_runs_small_scenario_with_priority(void) {
  assert_run_writes_csv("priority", priority_path);
}

void test_runs_small_scenario_with_round_robin(void) {
  assert_run_writes_csv("round-robin", round_robin_path);
}

void test_tick_limit_includes_context_switch_cost(void) {
  CliOptions options = {"fcfs", 42U, switch_config_path, fcfs_path};
  char error[256];

  write_switch_config();
  TEST_ASSERT_TRUE_MESSAGE(run_simulator(&options, error, sizeof(error)),
                           error);
}

void test_reports_configuration_and_output_failures(void) {
  CliOptions missing = {"fcfs", 42U, "configs/missing.conf", fcfs_path};
  CliOptions bad_output = {"fcfs", 42U, "configs/small.conf",
                           "tests/missing/run.csv"};
  char error[256];

  TEST_ASSERT_FALSE(run_simulator(&missing, error, sizeof(error)));
  TEST_ASSERT_NOT_EQUAL(0, error[0]);
  TEST_ASSERT_FALSE(run_simulator(&bad_output, error, sizeof(error)));
  TEST_ASSERT_NOT_EQUAL(0, error[0]);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_runs_small_scenario_with_fcfs);
  RUN_TEST(test_runs_small_scenario_with_priority);
  RUN_TEST(test_runs_small_scenario_with_round_robin);
  RUN_TEST(test_tick_limit_includes_context_switch_cost);
  RUN_TEST(test_reports_configuration_and_output_failures);
  return UNITY_END();
}
