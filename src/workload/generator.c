#include "workload.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static unsigned int lcg_rand(unsigned int *seed_state) {
  *seed_state = *seed_state * 1664525U + 1013904223U;
  return *seed_state;
}

static int rand_range(unsigned int *seed_state, int min, int max) {
  uint64_t span = (uint64_t)((int64_t)max - (int64_t)min + 1);
  return min + (int)(lcg_rand(seed_state) % span);
}

static bool config_is_valid(const ScenarioConfig *config) {
  return config != NULL && config->total_processes > 0 &&
         config->min_arrival >= 0 &&
         config->min_arrival <= config->max_arrival &&
         config->min_priority >= 0 &&
         config->min_priority <= config->max_priority &&
         config->high_priority_ratio >= 0.0 &&
         config->high_priority_ratio <= 1.0 &&
         config->min_cpu_burst_duration > 0 &&
         config->min_cpu_burst_duration <= config->max_cpu_burst_duration &&
         config->min_io_burst_duration >= 0 &&
         config->min_io_burst_duration <= config->max_io_burst_duration &&
         config->min_cpu_bursts > 0 &&
         config->min_cpu_bursts <= config->max_cpu_bursts &&
         config->rescue_interval > 0;
}

void free_workload(Process **workload, int total_processes) {
  if (workload == NULL)
    return;

  for (int i = 0; i < total_processes; i++) {
    if (workload[i] == NULL)
      continue;
    free(workload[i]->cpu_bursts);
    free(workload[i]->io_bursts);
    free(workload[i]);
  }
  free(workload);
}

Process **generate_workload(const ScenarioConfig *config, unsigned int seed) {
  if (!config_is_valid(config))
    return NULL;

  Process **workload =
      (Process **)calloc((size_t)config->total_processes, sizeof(Process *));
  if (workload == NULL)
    return NULL;

  unsigned int current_seed = seed;

  for (int i = 0; i < config->total_processes; i++) {
    Process *process = (Process *)calloc(1, sizeof(Process));
    if (process == NULL) {
      free_workload(workload, config->total_processes);
      return NULL;
    }
    workload[i] = process;

    process->id = i;
    process->arrival_time =
        rand_range(&current_seed, config->min_arrival, config->max_arrival);

    double prob = (double)(lcg_rand(&current_seed) % 1000) / 1000.0;
    if (prob < config->high_priority_ratio) {
      process->priority = config->min_priority;
    } else {
      process->priority = config->max_priority;
    }

    process->state = STATE_NEW;
    process->num_bursts = rand_range(&current_seed, config->min_cpu_bursts,
                                     config->max_cpu_bursts);

    if ((size_t)process->num_bursts > SIZE_MAX / sizeof(int)) {
      free_workload(workload, config->total_processes);
      return NULL;
    }
    process->cpu_bursts =
        (int *)malloc((size_t)process->num_bursts * sizeof(int));
    if (process->cpu_bursts == NULL) {
      free_workload(workload, config->total_processes);
      return NULL;
    }

    if (process->num_bursts > 1) {
      if ((size_t)(process->num_bursts - 1) > SIZE_MAX / sizeof(int)) {
        free_workload(workload, config->total_processes);
        return NULL;
      }
      process->io_bursts =
          (int *)malloc((size_t)(process->num_bursts - 1) * sizeof(int));
      if (process->io_bursts == NULL) {
        free_workload(workload, config->total_processes);
        return NULL;
      }
    }

    for (int j = 0; j < process->num_bursts; j++) {
      process->cpu_bursts[j] =
          rand_range(&current_seed, config->min_cpu_burst_duration,
                     config->max_cpu_burst_duration);
      if (j < process->num_bursts - 1) {
        process->io_bursts[j] =
            rand_range(&current_seed, config->min_io_burst_duration,
                       config->max_io_burst_duration);
      }
    }
  }

  return workload;
}
