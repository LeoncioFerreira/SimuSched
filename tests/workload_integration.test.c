#include "fcfs.h"
#include "process.h"
#include "simulation_core.h"
#include "unity.h"
#include "workload.h"

void setUp(void) {}

void tearDown(void) {}

void test_generated_workload_runs_in_simulation_core(void) {
  ScenarioConfig config = {
      .total_processes = 2,
      .min_arrival = 0,
      .max_arrival = 0,
      .min_priority = 1,
      .max_priority = 2,
      .min_burst_duration = 1,
      .max_burst_duration = 1,
      .min_cpu_bursts = 2,
      .max_cpu_bursts = 2,
  };

  Process **processes = generate_workload(&config, 42);
  TEST_ASSERT_NOT_NULL(processes);

  Scheduler *scheduler = create_fcfs_scheduler(config.total_processes);
  TEST_ASSERT_NOT_NULL(scheduler);

  SimulationCore core;
  core_init(&core, processes, config.total_processes, scheduler, 0);

  int ticks = 0;
  while (!core_is_finished(&core) && ticks < 20) {
    core_tick(&core);
    ticks++;
  }

  TEST_ASSERT_TRUE(core_is_finished(&core));
  TEST_ASSERT_EQUAL_INT(config.total_processes, core.completed_processes);

  core_destroy(&core);
  free_workload(processes, config.total_processes);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_generated_workload_runs_in_simulation_core);
  return UNITY_END();
}
