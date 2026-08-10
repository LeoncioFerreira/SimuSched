#include "workload.h"
#include <stdlib.h>

// Gerador de números pseudoaleatórios isolado (LCG)
static unsigned int lcg_rand(unsigned int *seed_state) {
  *seed_state = (*seed_state * 1103515245 + 12345) % ((unsigned int)1 << 31);
  return *seed_state;
}

// Gera intervalo [min, max] usando a seed passada
static int rand_range(unsigned int *seed_state, int min, int max) {
  if (min == max)
    return min;
  return min + (lcg_rand(seed_state) % (max - min + 1));
}

Process *generate_workload(ScenarioConfig config, unsigned int seed) {
  Process *workload =
      (Process *)malloc(config.total_processes * sizeof(Process));
  if (!workload)
    return NULL;

  unsigned int current_seed = seed;

  for (int i = 0; i < config.total_processes; i++) {
    workload[i].id = i;
    workload[i].arrival_time =
        rand_range(&current_seed, config.min_arrival, config.max_arrival);
    workload[i].priority =
        rand_range(&current_seed, config.min_priority, config.max_priority);
    workload[i].state = STATE_NEW;

    // Sorteia rajadas de CPU. Rajadas de IO serão N-1. Total = 2*N - 1.
    int num_cpu_bursts =
        rand_range(&current_seed, config.min_cpu_bursts, config.max_cpu_bursts);
    workload[i].num_bursts = num_cpu_bursts * 2 - 1;
    workload[i].bursts =
        (Burst *)malloc(workload[i].num_bursts * sizeof(Burst));

    // Montagem: CPU nos pares (0, 2, 4...), IO nos ímpares (1, 3, 5...)
    for (int j = 0; j < workload[i].num_bursts; j++) {
      workload[i].bursts[j].type = (j % 2 == 0) ? BURST_CPU : BURST_IO;
      workload[i].bursts[j].duration = rand_range(
          &current_seed, config.min_burst_duration, config.max_burst_duration);
    }
  }

  return workload;
}

void free_workload(Process *workload, int total_processes) {
  if (!workload)
    return;
  for (int i = 0; i < total_processes; i++) {
    free(workload[i].bursts);
  }
  free(workload);
}