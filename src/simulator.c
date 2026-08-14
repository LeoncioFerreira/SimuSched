#include "simulator.h"
#include "config_parser.h"
#include "csv_writer.h"
#include "fcfs.h"
#include "metrics.h"
#include "priority.h"
#include "round_robin.h"
#include "simulation_core.h"
#include "workload.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static bool fail(char *error, size_t error_size, const char *message) {
  if (error != NULL && error_size > 0)
    snprintf(error, error_size, "%s", message);
  return false;
}

static Scheduler *create_scheduler(const char *algorithm, int capacity) {
  if (strcmp(algorithm, "fcfs") == 0)
    return create_fcfs_scheduler(capacity);
  if (strcmp(algorithm, "priority") == 0)
    return create_priority_scheduler();
  if (strcmp(algorithm, "round-robin") == 0)
    return create_round_robin_scheduler(capacity);
  return NULL;
}

static bool calculate_tick_limit(Process **processes, int total_processes,
                                 int context_switch_cost, int *limit) {
  long long total = (long long)total_processes + 1;
  long long total_cpu_time = 0;

  for (int i = 0; i < total_processes; i++) {
    Process *process = processes[i];
    if (process->arrival_time > total)
      total = process->arrival_time + (long long)total_processes + 1;
    for (int j = 0; j < process->num_bursts; j++) {
      total += process->cpu_bursts[j];
      total_cpu_time += process->cpu_bursts[j];
      if (j < process->num_bursts - 1)
        total += process->io_bursts[j];
      if (total > INT_MAX)
        return false;
    }
  }
  if (total_cpu_time > 0)
    total += (total_cpu_time - 1) * context_switch_cost;
  if (total > INT_MAX)
    return false;
  *limit = (int)total;
  return true;
}

bool run_simulator(const CliOptions *options, char *error, size_t error_size) {
  Scenario scenario;
  Process **workload = NULL;
  Scheduler *scheduler = NULL;
  SimulationCore core;
  RunMetadata metadata = {0};
  int tick_limit;
  bool core_initialized = false;
  bool success = false;
  int quantum = 0;

  if (options == NULL || options->algorithm == NULL ||
      options->config_path == NULL || options->output_path == NULL)
    return fail(error, error_size, "opcoes de execucao invalidas");
  if (config_parse_file(options->config_path, &scenario, error, error_size) !=
      CONFIG_PARSE_OK)
    return false;

  workload = generate_workload(&scenario.config, options->seed);
  if (workload == NULL) {
    fail(error, error_size, "falha ao gerar a carga de processos");
    goto cleanup;
  }
  if (!calculate_tick_limit(workload, scenario.config.total_processes,
                            scenario.config.context_switch_cost, &tick_limit)) {
    fail(error, error_size, "carga excede o limite de tempo suportado");
    goto cleanup;
  }

  scheduler =
      create_scheduler(options->algorithm, scenario.config.total_processes);
  if (scheduler == NULL) {
    fail(error, error_size, "falha ao criar o escalonador");
    goto cleanup;
  }

  if (strcmp(options->algorithm, "round-robin") == 0)
    quantum = scenario.quantum;
  if (!core_init(&core, workload, scenario.config.total_processes, scheduler,
                 quantum, scenario.config.context_switch_cost)) {
    fail(error, error_size, "parametros invalidos para o nucleo da simulacao");
    goto cleanup;
  }
  core_initialized = true;
  scheduler = NULL;
  if (core.blocked_queue == NULL || core.blocked_queue->data == NULL) {
    fail(error, error_size, "falha ao inicializar o nucleo da simulacao");
    goto cleanup;
  }

  while (!core_is_finished(&core) && core.current_time < tick_limit)
    core_tick(&core);
  if (!core_is_finished(&core)) {
    fail(error, error_size, "simulacao nao terminou no limite calculado");
    goto cleanup;
  }

  metadata.algorithm = options->algorithm;
  metadata.seed = options->seed;
  metadata.scenario = scenario.name;
  metadata.configuration = options->config_path;
  metadata.total_processes = scenario.config.total_processes;
  metadata.total_simulated_time = core.current_time;
  metadata.quantum = scenario.quantum;
  metadata.context_switch_cost = scenario.config.context_switch_cost;
  metadata.average_turnaround =
      calculate_average_turnaround(workload, scenario.config.total_processes);
  metadata.context_switches = core.total_context_switches;
  metadata.jain_slowdown =
      calculate_jain_slowdown(workload, scenario.config.total_processes);
  if (!csv_write_run(options->output_path, &metadata, error, error_size))
    goto cleanup;
  success = true;

cleanup:
  if (core_initialized)
    core_destroy(&core);
  else if (scheduler != NULL)
    scheduler_destroy(scheduler);
  free_workload(workload, scenario.config.total_processes);
  return success;
}
