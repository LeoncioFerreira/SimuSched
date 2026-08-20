#include "cli.h"
#include "unity.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void assert_parse_fails(int argc, char **argv) {
  CliOptions options;
  char error[256];
  TEST_ASSERT_EQUAL_INT(CLI_PARSE_ERROR,
                        cli_parse(argc, argv, &options, error, sizeof(error)));
  TEST_ASSERT_NOT_EQUAL(0, error[0]);
}

void test_accepts_required_options_in_any_order(void) {
  char *argv[] = {
      "simulador", "--output",           "run.csv",     "--seed", "42",
      "--config",  "configs/small.conf", "--algorithm", "fcfs"};
  CliOptions options;
  char error[256];

  TEST_ASSERT_EQUAL_INT(CLI_PARSE_OK,
                        cli_parse(9, argv, &options, error, sizeof(error)));
  TEST_ASSERT_EQUAL_STRING("fcfs", options.algorithm);
  TEST_ASSERT_EQUAL_UINT(42U, options.seed);
  TEST_ASSERT_EQUAL_STRING("configs/small.conf", options.config_path);
  TEST_ASSERT_EQUAL_STRING("run.csv", options.output_path);
}

void test_rejects_missing_option(void) {
  char *argv[] = {"simulador", "--algorithm", "fcfs",      "--seed",
                  "42",        "--config",    "small.conf"};
  assert_parse_fails(7, argv);
}

void test_rejects_unknown_option(void) {
  char *argv[] = {"simulador", "--algorithm", "fcfs",      "--seed", "42",
                  "--config",  "small.conf",  "--unknown", "run.csv"};
  assert_parse_fails(9, argv);
}

void test_rejects_duplicate_option(void) {
  char *argv[] = {"simulador",  "--algorithm", "fcfs",   "--algorithm",
                  "priority",   "--seed",      "42",     "--config",
                  "small.conf", "--output",    "run.csv"};
  assert_parse_fails(11, argv);
}

void test_rejects_invalid_seeds(void) {
  char *negative[] = {"simulador", "--algorithm", "fcfs",     "--seed", "-1",
                      "--config",  "small.conf",  "--output", "run.csv"};
  char *text[] = {"simulador", "--algorithm", "fcfs",     "--seed", "abc",
                  "--config",  "small.conf",  "--output", "run.csv"};
  char too_large[64];
  snprintf(too_large, sizeof(too_large), "%llu",
           (unsigned long long)UINT_MAX + 1ULL);
  char *large[] = {"simulador", "--algorithm", "fcfs",     "--seed", too_large,
                   "--config",  "small.conf",  "--output", "run.csv"};

  assert_parse_fails(9, negative);
  assert_parse_fails(9, text);
  assert_parse_fails(9, large);
}

void test_rejects_unknown_algorithm(void) {
  char *argv[] = {"simulador", "--algorithm", "sjf",      "--seed", "42",
                  "--config",  "small.conf",  "--output", "run.csv"};
  assert_parse_fails(9, argv);
}

void test_accepts_round_robin_algorithm(void) {
  CliOptions options;
  char error[256];
  char *argv[] = {"simulador", "--algorithm", "round-robin", "--seed", "42",
                  "--config",  "small.conf",  "--output",    "run.csv"};

  TEST_ASSERT_EQUAL_INT(CLI_PARSE_OK,
                        cli_parse(9, argv, &options, error, sizeof(error)));
  TEST_ASSERT_EQUAL_STRING("round-robin", options.algorithm);
}

void test_accepts_tej_algorithm(void) {
  CliOptions options;
  char error[256];
  char *argv[] = {"simulador", "--algorithm", "tej",      "--seed", "42",
                  "--config",  "small.conf",  "--output", "run.csv"};

  TEST_ASSERT_EQUAL_INT(CLI_PARSE_OK,
                        cli_parse(9, argv, &options, error, sizeof(error)));
  TEST_ASSERT_EQUAL_STRING("tej", options.algorithm);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_accepts_required_options_in_any_order);
  RUN_TEST(test_rejects_missing_option);
  RUN_TEST(test_rejects_unknown_option);
  RUN_TEST(test_rejects_duplicate_option);
  RUN_TEST(test_rejects_invalid_seeds);
  RUN_TEST(test_rejects_unknown_algorithm);
  RUN_TEST(test_accepts_round_robin_algorithm);
  RUN_TEST(test_accepts_tej_algorithm);
  return UNITY_END();
}
