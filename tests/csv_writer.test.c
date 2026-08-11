#include "csv_writer.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>

static const char *test_path = "tests/tmp_run.csv";

void setUp(void) { remove(test_path); }
void tearDown(void) { remove(test_path); }

void test_writes_header_and_run_metadata(void) {
  RunMetadata metadata = {"fcfs", 42U, "small", 5, 37};
  char error[256];
  char content[256];
  FILE *file;

  TEST_ASSERT_TRUE(csv_write_run(test_path, &metadata, error, sizeof(error)));
  file = fopen(test_path, "r");
  TEST_ASSERT_NOT_NULL(file);
  TEST_ASSERT_NOT_NULL(fgets(content, sizeof(content), file));
  TEST_ASSERT_EQUAL_STRING(
      "algorithm,seed,scenario,total_processes,total_simulated_time\n",
      content);
  TEST_ASSERT_NOT_NULL(fgets(content, sizeof(content), file));
  TEST_ASSERT_EQUAL_STRING("fcfs,42,small,5,37\n", content);
  TEST_ASSERT_NULL(fgets(content, sizeof(content), file));
  fclose(file);
}

void test_rejects_invalid_path_and_null_metadata(void) {
  RunMetadata metadata = {"fcfs", 42U, "small", 5, 37};
  char error[256];
  TEST_ASSERT_FALSE(
      csv_write_run("tests/missing/run.csv", &metadata, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, NULL, error, sizeof(error)));
}

void test_rejects_unsafe_text_fields(void) {
  RunMetadata comma = {"fcfs,priority", 42U, "small", 5, 37};
  RunMetadata quote = {"fcfs", 42U, "small\"name", 5, 37};
  RunMetadata newline = {"fcfs", 42U, "small\nname", 5, 37};
  char error[256];
  TEST_ASSERT_FALSE(csv_write_run(test_path, &comma, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, &quote, error, sizeof(error)));
  TEST_ASSERT_FALSE(csv_write_run(test_path, &newline, error, sizeof(error)));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_writes_header_and_run_metadata);
  RUN_TEST(test_rejects_invalid_path_and_null_metadata);
  RUN_TEST(test_rejects_unsafe_text_fields);
  return UNITY_END();
}
