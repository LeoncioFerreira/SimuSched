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
         "high_priority_ratio=0.5\n"
         "min_cpu_burst_duration=1\n"
         "max_cpu_burst_duration=8\n"
         "min_io_burst_duration=2\n"
         "max_io_burst_duration=4\n"
         "min_cpu_bursts=1\n"
         "max_cpu_bursts=3\n"
         "quantum=2\n"
         "context_switch_cost=1\n"
         "rescue_interval=10\n";
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
  TEST_ASSERT_TRUE(scenario.config.high_priority_ratio > 0.49 &&
                   scenario.config.high_priority_ratio < 0.51);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.min_cpu_burst_duration);
  TEST_ASSERT_EQUAL_INT(8, scenario.config.max_cpu_burst_duration);
  TEST_ASSERT_EQUAL_INT(2, scenario.config.min_io_burst_duration);
  TEST_ASSERT_EQUAL_INT(4, scenario.config.max_io_burst_duration);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.min_cpu_bursts);
  TEST_ASSERT_EQUAL_INT(3, scenario.config.max_cpu_bursts);
  TEST_ASSERT_EQUAL_INT(2, scenario.quantum);
  TEST_ASSERT_EQUAL_INT(1, scenario.config.context_switch_cost);
  TEST_ASSERT_EQUAL_INT(10, scenario.config.rescue_interval);
}

void test_rejects_non_positive_rescue_interval(void) {
  char config[1024];
  snprintf(config, sizeof(config), "%s", valid_config());
  char *interval = strstr(config, "rescue_interval=10");
  TEST_ASSERT_NOT_NULL(interval);
  strcpy(interval, "rescue_interval=0\n");
  assert_config_fails(config);

  snprintf(config, sizeof(config), "%s", valid_config());
  interval = strstr(config, "rescue_interval=10");
  TEST_ASSERT_NOT_NULL(interval);
  strcpy(interval, "rescue_interval=-1\n");
  assert_config_fails(config);
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

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_parses_valid_scenario);
  RUN_TEST(test_rejects_non_positive_rescue_interval);
  RUN_TEST(test_rejects_missing_file);
  RUN_TEST(test_rejects_unknown_and_duplicate_keys);
  RUN_TEST(test_rejects_missing_and_malformed_keys);
  return UNITY_END();
}
