/**
 * Descrição: Suíte de testes unitários utilizando o framework Unity para
 * validar as transições do ciclo de vida dos processos na simulação.
 * Autor: Leôncio Ferreira
 */
#include "fcfs.h"
#include "priority.h"
#include "simulation_core.h"
#include "unity.h"
#include <stdlib.h>

SimulationCore core;
Process *processos[2];
int cpu_bursts_p1[] = {2, 1};
int io_bursts_p1[] = {3};

int cpu_bursts_p2[] = {1};
int io_bursts_p2[] = {0};

void setUp(void) {
  processos[0] = (Process *)malloc(sizeof(Process));
  processos[0]->id = 1;
  processos[0]->arrival_time = 0;
  processos[0]->state = STATE_NEW;
  processos[0]->cpu_bursts = cpu_bursts_p1;
  processos[0]->io_bursts = io_bursts_p1;
  processos[0]->num_bursts = 2;
  processos[0]->current_burst_index = 0;
  processos[0]->remaining_burst_time = 0;

  processos[1] = (Process *)malloc(sizeof(Process));
  processos[1]->id = 2;
  processos[1]->arrival_time = 2;
  processos[1]->state = STATE_NEW;
  processos[1]->cpu_bursts = cpu_bursts_p2;
  processos[1]->io_bursts = io_bursts_p2;
  processos[1]->num_bursts = 1;
  processos[1]->current_burst_index = 0;
  processos[1]->remaining_burst_time = 0;

  Scheduler *sched = create_fcfs_scheduler(1000);
  core_init(&core, processos, 2, sched);
}

void tearDown(void) {
  core_destroy(&core);
  free(processos[0]);
  free(processos[1]);
}

void test_process_arrival(void) {
  // Tick 0: P1 chega e é escalonado. Consome 1 tick (resta 1)
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT(1, core.current_time);
  TEST_ASSERT_EQUAL(STATE_RUNNING, processos[0]->state);

  // Tick 1: P2 ainda não chegou. P1 consome mais 1 tick (resta 0) -> BLOCKED
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_NEW, processos[1]->state);

  // Tick 2: P2 chega (READY). P1 já está BLOCKED.
  // Como a CPU está livre, P2 entra na CPU. Como P2 só tem 1 de burst, ele
  // termina agora!
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_FINISHED, processos[1]->state);
}

void test_process_blocking_and_unblocking(void) {
  // P1 tem 2 de CPU e 3 de IO.
  core_tick(&core); // Tick 0 -> 1: P1 roda (resta 1)
  core_tick(&core); // Tick 1 -> 2: P1 roda (resta 0). Vai para BLOCKED.

  TEST_ASSERT_EQUAL(STATE_BLOCKED, processos[0]->state);
  TEST_ASSERT_EQUAL_INT(3, processos[0]->remaining_burst_time); // IO burst de 3

  core_tick(&core); // Tick 2 -> 3: P1 em IO (faltam 2). P2 roda e finaliza.
  core_tick(&core); // Tick 3 -> 4: P1 em IO (falta 1)

  // Tick 4 -> 5: P1 em IO (falta 0), vira READY. (agora consome o tick 4 de
  // verdade no IO)
  core_tick(&core);

  // Tick 5 -> 6: P1 roda e FINALIZA
  core_tick(&core);
  TEST_ASSERT_EQUAL(STATE_FINISHED, processos[0]->state);
}

void test_process_finish(void) {
  // Vamos pular os passos do bloqueio e checar o fim do P1
  for (int i = 0; i < 6; i++)
    core_tick(&core);

  // P1 terminou e deve ter a flag finish salva em current_time
  TEST_ASSERT_EQUAL(STATE_FINISHED, processos[0]->state);
  TEST_ASSERT_EQUAL_INT(6, processos[0]->finish_time);

  // Verifica se a flag global completou os 2 (P1 e P2)
  TEST_ASSERT_EQUAL_INT(2, core.completed_processes);
}

void test_priority_integration(void) {
  SimulationCore local_core;
  Process *local_procs[2];
  local_procs[0] = (Process *)malloc(sizeof(Process));
  local_procs[0]->id = 1;
  local_procs[0]->arrival_time = 0;
  local_procs[0]->state = STATE_NEW;
  local_procs[0]->cpu_bursts = cpu_bursts_p1;
  local_procs[0]->io_bursts = io_bursts_p1;
  local_procs[0]->num_bursts = 2;
  local_procs[0]->current_burst_index = 0;
  local_procs[0]->remaining_burst_time = 0;
  local_procs[0]->priority = 10;

  local_procs[1] = (Process *)malloc(sizeof(Process));
  local_procs[1]->id = 2;
  local_procs[1]->arrival_time = 1;
  local_procs[1]->state = STATE_NEW;
  local_procs[1]->cpu_bursts = cpu_bursts_p2;
  local_procs[1]->io_bursts = io_bursts_p2;
  local_procs[1]->num_bursts = 1;
  local_procs[1]->current_burst_index = 0;
  local_procs[1]->remaining_burst_time = 0;
  local_procs[1]->priority = 5;

  Scheduler *prio_sched = create_priority_scheduler();
  core_init(&local_core, local_procs, 2, prio_sched);

  core_tick(&local_core); // Tick 0: p1 arrives and runs (burst 2)
  TEST_ASSERT_EQUAL(STATE_RUNNING, local_procs[0]->state);

  core_tick(&local_core); // Tick 1: p1 goes to IO, CPU free, p2 NOT running yet
                          // (will be picked next tick)
  TEST_ASSERT_EQUAL(STATE_BLOCKED, local_procs[0]->state);
  TEST_ASSERT_EQUAL(STATE_READY, local_procs[1]->state);

  core_tick(&local_core); // Tick 2: p2 runs and finishes (CPU burst of 1)
  TEST_ASSERT_EQUAL(STATE_BLOCKED, local_procs[0]->state);
  TEST_ASSERT_EQUAL(STATE_FINISHED, local_procs[1]->state);

  core_destroy(&local_core);
  free(local_procs[0]);
  free(local_procs[1]);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_process_arrival);
  RUN_TEST(test_process_blocking_and_unblocking);
  RUN_TEST(test_process_finish);
  RUN_TEST(test_priority_integration);
  return UNITY_END();
}
