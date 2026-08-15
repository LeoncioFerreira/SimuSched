#include "csv_writer.h"
#include "unity.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *test_path = "tests/tmp_run.csv";

void setUp(void) { remove(test_path); }
void tearDown(void) { remove(test_path); }

void test_writes_header_and_run_metadata(void) {
  RunMetadata metadata = {
      .algorithm = "fcfs",
      .seed = 42U,
      .scenario = "small",
      .configuration = "configs/small.conf",
      .total_processes = 5,
      .total_simulated_time = 37,
      .quantum = 2,
      .context_switch_cost = 1,
      .rescue_interval = 10,
      .average_turnaround = 12.5,
      .context_switches = 4,
      .jain_slowdown = 0.875,
  };
  char error[256];
  char content[512];
  FILE *file;

  TEST_ASSERT_TRUE(csv_write_run(test_path, &metadata, error, sizeof(error)));
  file = fopen(test_path, "r");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_NOT_NULL(fgets(content, sizeof(content), file));
  TEST_ASSERT_EQUAL_STRING(
      "algorithm,seed,scenario,configuration,total_processes,"
      "total_simulated_time,quantum,context_switch_cost,rescue_interval,"
      "average_turnaround,"
      "context_switches,jain_slowdown\n",
      content);
  TEST_ASSERT_NOT_NULL(fgets(content, sizeof(content), file));
  TEST_ASSERT_EQUAL_STRING(
      "fcfs,42,small,configs/small.conf,5,37,2,1,10,12.500000,4,0.875000\n",
      content);
  TEST_ASSERT_NULL(fgets(content, sizeof(content), file));
  fclose(file);
}

void test_rejects_invalid_path_and_null_metadata(void) {
  RunMetadata metadata = {
      .algorithm = "fcfs",
      .seed = 42U,
      .scenario = "small",
      .configuration = "configs/small.conf",
      .total_processes = 5,
      .total_simulated_time = 37,
      .quantum = 2,
      .context_switch_cost = 1,
      .rescue_interval = 10,
      .average_turnaround = 12.5,
      .context_switches = 4,
      .jain_slowdown = 0.875,
  };
  char error[256];
  TEST_ASSERT_FALSE(
      csv_write_run("tests/missing/run.csv", &metadata, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, NULL, error, sizeof(error)));
}

void test_rejects_unsafe_text_fields(void) {
  RunMetadata valid = {
      .algorithm = "fcfs",
      .seed = 42U,
      .scenario = "small",
      .configuration = "configs/small.conf",
      .total_processes = 5,
      .total_simulated_time = 37,
      .quantum = 2,
      .context_switch_cost = 1,
      .rescue_interval = 10,
      .average_turnaround = 12.5,
      .context_switches = 4,
      .jain_slowdown = 0.875,
  };
  RunMetadata comma = valid;
  RunMetadata quote = valid;
  RunMetadata newline = valid;
  RunMetadata unsafe_configuration = valid;
  char error[256];
  comma.algorithm = "fcfs,priority";
  quote.scenario = "small\"name";
  newline.scenario = "small\nname";
  unsafe_configuration.configuration = "configs/small,large.conf";
  TEST_ASSERT_FALSE(csv_write_run(test_path, &comma, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, &quote, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, &newline, error, sizeof(error)));
  TEST_ASSERT_FALSE(
      csv_write_run(test_path, &unsafe_configuration, error, sizeof(error)));
}

void test_rejects_non_finite_metrics(void) {
  RunMetadata valid = {
      .algorithm = "fcfs",
      .seed = 42U,
      .scenario = "small",
      .configuration = "configs/small.conf",
      .total_processes = 5,
      .total_simulated_time = 37,
      .quantum = 2,
      .context_switch_cost = 1,
      .rescue_interval = 10,
      .average_turnaround = 12.5,
      .context_switches = 4,
      .jain_slowdown = 0.875,
  };
  RunMetadata nan_turnaround = valid;
  RunMetadata nan_jain = valid;
  RunMetadata infinite_jain = valid;
  RunMetadata invalid_rescue_interval = valid;
  char error[256];
  nan_turnaround.average_turnaround = NAN;
  nan_jain.jain_slowdown = NAN;
  infinite_jain.jain_slowdown = INFINITY;
  invalid_rescue_interval.rescue_interval = 0;

  TEST_ASSERT_FALSE(
      csv_write_run(test_path, &nan_turnaround, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, &nan_jain, error, sizeof(error)));
  TEST_ASSERT_FALSE(
      csv_write_run(test_path, &infinite_jain, error, sizeof(error)));
  TEST_ASSERT_FALSE(
      csv_write_run(test_path, &invalid_rescue_interval, error, sizeof(error)));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_writes_header_and_run_metadata);
  RUN_TEST(test_rejects_invalid_path_and_null_metadata);
  RUN_TEST(test_rejects_unsafe_text_fields);
  RUN_TEST(test_rejects_non_finite_metrics);
  return UNITY_END();
}
