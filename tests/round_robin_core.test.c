/**
 * Descrição: Testes de integração entre o núcleo da simulação e o Round Robin,
 * cobrindo revezamento, processo único, chegada e retorno de Entrada/Saída.
 * Autor: Leôncio Ferreira
 */
#include "round_robin.h"
#include "simulation_core.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

static Process process_with_bursts(int id, int arrival_time, int *cpu_bursts,
                                   int *io_bursts, int num_bursts) {
  Process process = {
      .id = id,
      .arrival_time = arrival_time,
      .state = STATE_NEW,
      .cpu_bursts = cpu_bursts,
      .io_bursts = io_bursts,
      .num_bursts = num_bursts,
      .current_burst_index = 0,
      .remaining_burst_time = 0,
  };
  return process;
}

void test_preempts_and_rotates_processes_after_quantum(void) {
  int first_cpu[] = {4};
  int second_cpu[] = {4};
  Process first = process_with_bursts(1, 0, first_cpu, NULL, 1);
  Process second = process_with_bursts(2, 0, second_cpu, NULL, 1);
  Process *processes[] = {&first, &second};
  SimulationCore core;

  core_init(&core, processes, 2, create_round_robin_scheduler(2), 2);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_NULL(core.running_process);
  TEST_ASSERT_EQUAL(STATE_READY, first.state);
  core_tick(&core);
  TEST_ASSERT_EQUAL_PTR(&second, core.running_process);

  core_destroy(&core);
}

void test_keeps_single_process_running_after_quantum(void) {
  int cpu[] = {4};
  Process process = process_with_bursts(1, 0, cpu, NULL, 1);
  Process *processes[] = {&process};
  SimulationCore core;

  core_init(&core, processes, 1, create_round_robin_scheduler(1), 2);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_EQUAL_PTR(&process, core.running_process);
  TEST_ASSERT_EQUAL(STATE_RUNNING, process.state);
  TEST_ASSERT_EQUAL_INT(0, core.quantum_used);

  core_destroy(&core);
}

void test_preempts_when_process_arrives_during_execution(void) {
  int first_cpu[] = {4};
  int second_cpu[] = {1};
  Process first = process_with_bursts(1, 0, first_cpu, NULL, 1);
  Process second = process_with_bursts(2, 1, second_cpu, NULL, 1);
  Process *processes[] = {&first, &second};
  SimulationCore core;

  core_init(&core, processes, 2, create_round_robin_scheduler(2), 2);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_NULL(core.running_process);
  TEST_ASSERT_EQUAL(STATE_READY, first.state);
  TEST_ASSERT_EQUAL(STATE_READY, second.state);
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_FINISHED, second.state);

  core_destroy(&core);
}

void test_unblocked_process_returns_to_end_of_ready_queue(void) {
  int first_cpu[] = {1, 1};
  int first_io[] = {1};
  int second_cpu[] = {3};
  int third_cpu[] = {1};
  Process first = process_with_bursts(1, 0, first_cpu, first_io, 2);
  Process second = process_with_bursts(2, 0, second_cpu, NULL, 1);
  Process third = process_with_bursts(3, 1, third_cpu, NULL, 1);
  Process *processes[] = {&first, &second, &third};
  SimulationCore core;

  core_init(&core, processes, 3, create_round_robin_scheduler(3), 1);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_EQUAL(STATE_READY, first.state);
  TEST_ASSERT_EQUAL(STATE_READY, second.state);
  TEST_ASSERT_EQUAL(STATE_READY, third.state);
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_FINISHED, third.state);
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_READY, first.state);
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_FINISHED, first.state);

  core_destroy(&core);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_preempts_and_rotates_processes_after_quantum);
  RUN_TEST(test_keeps_single_process_running_after_quantum);
  RUN_TEST(test_preempts_when_process_arrives_during_execution);
  RUN_TEST(test_unblocked_process_returns_to_end_of_ready_queue);
  return UNITY_END();
}
