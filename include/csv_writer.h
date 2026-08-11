#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char *algorithm;
  unsigned int seed;
  const char *scenario;
  int total_processes;
  int total_simulated_time;
} RunMetadata;

bool csv_write_run(const char *path, const RunMetadata *metadata, char *error,
                   size_t error_size);

#endif
