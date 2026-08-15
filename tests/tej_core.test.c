/**
 * Descrição: Valida a integração não preemptiva do TEJ com o núcleo e o
 * reinício do período de espera quando um processo retorna de E/S.
 * Autor: Leôncio Ferreira
 */
#include "simulation_core.h"
#include "tej.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_does_not_preempt_running_cpu_burst(void) {
  int long_cpu[] = {3};
  int urgent_cpu[] = {1};
  Process running = {.id = 1,
                     .priority = 3,
                     .arrival_time = 0,
                     .state = STATE_NEW,
                     .cpu_bursts = long_cpu,
                     .num_bursts = 1};
  Process urgent = {.id = 2,
                    .priority = 0,
                    .arrival_time = 1,
                    .state = STATE_NEW,
                    .cpu_bursts = urgent_cpu,
                    .num_bursts = 1};
  Process *processes[] = {&running, &urgent};
  SimulationCore core;

  TEST_ASSERT_TRUE(
      core_init(&core, processes, 2, create_tej_scheduler(2, 0, 10), 0, 0));
  core_tick(&core);
  core_tick(&core);
  TEST_ASSERT_EQUAL_PTR(&running, core.running_process);
  TEST_ASSERT_EQUAL_INT(1, running.remaining_burst_time);

  core_tick(&core);
  TEST_ASSERT_EQUAL_INT(STATE_FINISHED, running.state);
  TEST_ASSERT_EQUAL_INT(STATE_READY, urgent.state);
  core_destroy(&core);
}

void test_io_return_starts_a_new_ready_wait_period(void) {
  int first_cpu[] = {1, 1};
  int first_io[] = {2};
  int second_cpu[] = {4};
  Process io_process = {.id = 1,
                        .priority = 0,
                        .arrival_time = 0,
                        .state = STATE_NEW,
                        .cpu_bursts = first_cpu,
                        .io_bursts = first_io,
                        .num_bursts = 2};
  Process cpu_process = {.id = 2,
                         .priority = 2,
                         .arrival_time = 0,
                         .state = STATE_NEW,
                         .cpu_bursts = second_cpu,
                         .num_bursts = 1};
  Process *processes[] = {&io_process, &cpu_process};
  SimulationCore core;

  TEST_ASSERT_TRUE(
      core_init(&core, processes, 2, create_tej_scheduler(2, 0, 10), 0, 0));
  core_tick(&core);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_EQUAL_INT(STATE_READY, io_process.state);
  TEST_ASSERT_EQUAL_INT(3, io_process.ready_queue_arrival_time);
  core_destroy(&core);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_does_not_preempt_running_cpu_burst);
  RUN_TEST(test_io_return_starts_a_new_ready_wait_period);
  return UNITY_END();
}
