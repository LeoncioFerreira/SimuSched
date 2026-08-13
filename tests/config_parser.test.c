#include "config_parser.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>

static const char *test_path = "tests/tmp_scenario.conf";

void setUp(void) { remove(test_path); }
void tearDown(void) { remove(test_path); }

static void write_config(const char *content) {
  FILE *file = fopen(test_path, "w");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_TRUE(fputs(content, file) >= 0);
  TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

static const char *valid_config(void) {
  return "scenario=small\n"
         "total_processes=5\n"
         "min_arrival=0\n"
         "max_arrival=10\n"
         "min_priority=1\n"
         "max_priority=5\n"
         "min_burst_duration=1\n"
         "max_burst_duration=8\n"
         "min_cpu_bursts=1\n"
         "max_cpu_bursts=3\n"
         "quantum=2\n";
}

static void assert_config_fails(const char *content) {
  Scenario scenario;
  char error[256];
  write_config(content);
  TEST_ASSERT_EQUAL_INT(
      CONFIG_PARSE_ERROR,
      config_parse_file(test_path, &scenario, error, sizeof(error)));
  TEST_ASSERT_NOT_EQUAL(0, error[0]);
}

void test_parses_valid_scenario(void) {
  Scenario scenario;
  char error[256];
  write_config(valid_config());

  TEST_ASSERT_EQUAL_INT(
      CONFIG_PARSE_OK,
      config_parse_file(test_path, &scenario, error, sizeof(error)));
  TEST_ASSERT_EQUAL_STRING("small", scenario.name);
  TEST_ASSERT_EQUAL_INT(5, scenario.config.total_processes);
  TEST_ASSERT_EQUAL_INT(0, scenario.config.min_arrival);
  TEST_ASSERT_EQUAL_INT(10, scenario.config.max_arrival);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.min_priority);
  TEST_ASSERT_EQUAL_INT(5, scenario.config.max_priority);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.min_burst_duration);
  TEST_ASSERT_EQUAL_INT(8, scenario.config.max_burst_duration);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.min_cpu_bursts);
  TEST_ASSERT_EQUAL_INT(3, scenario.config.max_cpu_bursts);
  TEST_ASSERT_EQUAL_INT(2, scenario.quantum);
}

void test_rejects_invalid_quantum(void) {
  assert_config_fails("scenario=small\n"
                      "total_processes=5\n"
                      "min_arrival=0\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n"
                      "quantum=0\n");
  assert_config_fails("scenario=small\n"
                      "total_processes=5\n"
                      "min_arrival=0\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n"
                      "quantum=-1\n");
  assert_config_fails("scenario=small\n"
                      "total_processes=5\n"
                      "min_arrival=0\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n"
                      "quantum=abc\n");
}

void test_rejects_missing_file(void) {
  Scenario scenario;
  char error[256];
  TEST_ASSERT_EQUAL_INT(CONFIG_PARSE_ERROR,
                        config_parse_file("tests/does-not-exist.conf",
                                          &scenario, error, sizeof(error)));
}

void test_rejects_unknown_and_duplicate_keys(void) {
  char unknown[1024];
  char duplicate[1024];
  snprintf(unknown, sizeof(unknown), "%sunknown=1\n", valid_config());
  snprintf(duplicate, sizeof(duplicate), "%stotal_processes=6\n",
           valid_config());
  assert_config_fails(unknown);
  assert_config_fails(duplicate);
}

void test_rejects_missing_and_malformed_keys(void) {
  assert_config_fails("scenario=small\ntotal_processes=5\n");
  assert_config_fails("scenario small\n");
}

void test_rejects_invalid_values_and_ranges(void) {
  assert_config_fails("scenario=small\n"
                      "total_processes=0\n"
                      "min_arrival=0\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n");
  assert_config_fails("scenario=small\n"
                      "total_processes=5\n"
                      "min_arrival=11\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n");
  assert_config_fails("scenario=small\n"
                      "total_processes=five\n"
                      "min_arrival=0\nmax_arrival=10\n"
                      "min_priority=1\nmax_priority=5\n"
                      "min_burst_duration=1\nmax_burst_duration=8\n"
                      "min_cpu_bursts=1\nmax_cpu_bursts=3\n");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_parses_valid_scenario);
  RUN_TEST(test_rejects_invalid_quantum);
  RUN_TEST(test_rejects_missing_file);
  RUN_TEST(test_rejects_unknown_and_duplicate_keys);
  RUN_TEST(test_rejects_missing_and_malformed_keys);
  RUN_TEST(test_rejects_invalid_values_and_ranges);
  return UNITY_END();
}
