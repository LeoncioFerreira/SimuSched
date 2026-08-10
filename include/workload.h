#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stdbool.h>

typedef enum { BURST_CPU, BURST_IO } BurstType;
typedef enum { STATE_NEW, STATE_READY } ProcessState;

typedef struct {
  BurstType type;
  int duration;
} Burst;

typedef struct {
  int id;
  int arrival_time;
  int priority;
  ProcessState state;
  Burst *bursts;
  int num_bursts;
} Process;

// Recebe as configurações do cenário como entrada explícita
typedef struct {
  int total_processes;
  int min_arrival;
  int max_arrival;
  int min_priority;
  int max_priority;
  int min_burst_duration;
  int max_burst_duration;
  int min_cpu_bursts;
  int max_cpu_bursts;
} ScenarioConfig;

// Protótipos das funções
Process *generate_workload(ScenarioConfig config, unsigned int seed);
void free_workload(Process *workload, int total_processes);

#endif // WORKLOAD_H