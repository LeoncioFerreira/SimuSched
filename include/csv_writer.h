#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char *algorithm;
  unsigned int seed;
  const char *scenario;
  const char *configuration;
  int total_processes;
  int total_simulated_time;
  int quantum;
  int context_switch_cost;
  double average_turnaround;
  int context_switches;
  double jain_slowdown;
} RunMetadata;

bool csv_write_run(const char *path, const RunMetadata *metadata, char *error,
                   size_t error_size);

#endif
