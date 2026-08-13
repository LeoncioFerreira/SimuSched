#include "simulator.h"
#include "config_parser.h"
#include "csv_writer.h"
#include "fcfs.h"
#include "priority.h"
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
  return NULL;
}

static bool calculate_tick_limit(Process **processes, int total_processes,
                                 int *limit) {
  long long total = (long long)total_processes + 1;

  for (int i = 0; i < total_processes; i++) {
    Process *process = processes[i];
    if (process->arrival_time > total)
      total = process->arrival_time + (long long)total_processes + 1;
    for (int j = 0; j < process->num_bursts; j++) {
      total += process->cpu_bursts[j];
      if (j < process->num_bursts - 1)
        total += process->io_bursts[j];
      if (total > INT_MAX)
        return false;
    }
  }
  *limit = (int)total;
  return true;
}

bool run_simulator(const CliOptions *options, char *error, size_t error_size) {
  Scenario scenario;
  Process **workload = NULL;
  Scheduler *scheduler = NULL;
  SimulationCore core;
  RunMetadata metadata;
  int tick_limit;
  bool core_initialized = false;
  bool success = false;

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
                            &tick_limit)) {
    fail(error, error_size, "carga excede o limite de tempo suportado");
    goto cleanup;
  }

  scheduler =
      create_scheduler(options->algorithm, scenario.config.total_processes);
  if (scheduler == NULL) {
    fail(error, error_size, "falha ao criar o escalonador");
    goto cleanup;
  }

  core_init(&core, workload, scenario.config.total_processes, scheduler,
            scenario.config.context_switch_cost);
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
  metadata.total_processes = scenario.config.total_processes;
  metadata.total_simulated_time = core.current_time;
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
