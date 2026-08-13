#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "process.h"

typedef struct {
  int total_processes;
  int min_arrival;
  int max_arrival;
  int min_priority;
  int max_priority;
  double high_priority_ratio;
  int min_cpu_burst_duration;
  int max_cpu_burst_duration;
  int min_cpu_bursts;
  int max_cpu_bursts;
  int min_io_burst_duration;
  int max_io_burst_duration;
  int context_switch_cost;
} ScenarioConfig;

Process **generate_workload(const ScenarioConfig *config, unsigned int seed);
void free_workload(Process **workload, int total_processes);

#endif
