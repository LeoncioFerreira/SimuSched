#include "simulation_core.h"
#include "circular_queue.h"
#include "process.h"
#include "unity.h"
#include <stdlib.h>

// --- Mocks para o Scheduler ---
Process *mock_queue[10];
int mock_head = 0, mock_tail = 0;

bool mock_enqueue(Scheduler *self, Process *p) {
  (void)self;
  mock_queue[mock_tail++] = p;
  return true;
}

Process *mock_get_next(Scheduler *self) {
  (void)self;
  if (mock_head == mock_tail)
    return NULL;
  return mock_queue[mock_head++];
}

bool mock_is_empty(Scheduler *self) {
  (void)self;
  return mock_head == mock_tail;
}

Scheduler mock_scheduler = {.state = NULL,
                            .enqueue_process = mock_enqueue,
                            .get_next_process = mock_get_next,
                            .is_empty = mock_is_empty,
                            .destroy = NULL};

// Limpeza das filas do mock antes de cada teste
void setUp(void) {
  mock_head = 0;
  mock_tail = 0;
}
void tearDown(void) {}

// --- Testes dos Critérios de Aceite ---

void test_negative_switch_cost_validation(void) {
  SimulationCore core;
  bool init_result = core_init(&core, NULL, 0, &mock_scheduler, 0, -1);
  TEST_ASSERT_FALSE_MESSAGE(init_result,
                            "O core deve rejeitar custos de troca negativos");
}

void test_parallel_io_completion(void) {
  int p1_cpu[] = {1, 1}, p1_io[] = {2};
  Process p1 = {.id = 1,
                .state = STATE_BLOCKED,
                .remaining_burst_time = 2,
                .cpu_bursts = p1_cpu,
                .io_bursts = p1_io,
                .num_bursts = 2,
                .current_burst_index = 0};

  int p2_cpu[] = {1, 1}, p2_io[] = {3};
  Process p2 = {.id = 2,
                .state = STATE_BLOCKED,
                .remaining_burst_time = 3,
                .cpu_bursts = p2_cpu,
                .io_bursts = p2_io,
                .num_bursts = 2,
                .current_burst_index = 0};

  Process *procs[] = {&p1, &p2};
  SimulationCore core;

  // Inicializa com custo 0 para não interferir neste teste
  core_init(&core, procs, 2, &mock_scheduler, 0, 0);

  // Insere processos na fila de bloqueados simulando que entraram em I/O
  circular_queue_enqueue(core.blocked_queue, &p1);
  circular_queue_enqueue(core.blocked_queue, &p2);

  // Tick 1: p1_rem cai para 1, p2_rem cai para 2
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT(STATE_BLOCKED, p1.state);

  // Tick 2: p1_rem = 0 (Termina e vai pra ready), p2_rem = 1
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT_MESSAGE(STATE_READY, p1.state,
                                "Processo 1 deve concluir IO apos 2 ticks");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, p1.ready_queue_arrival_time,
                                "Instante de retorno deve estar documentado");
  TEST_ASSERT_EQUAL_INT(STATE_BLOCKED, p2.state);

  // Tick 3: p2_rem = 0 (Termina e vai pra ready)
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT_MESSAGE(STATE_READY, p2.state,
                                "Processo 2 deve concluir IO no mesmo "
                                "intervalo sem bloqueio de fila unica");

  core_destroy(&core);
}

void test_context_switch_cpu_unavailable(void) {
  int p1_cpu[] = {1}, p2_cpu[] = {5};
  Process p1 = {.id = 1,
                .state = STATE_NEW,
                .arrival_time = 0,
                .remaining_burst_time = 1,
                .cpu_bursts = p1_cpu,
                .num_bursts = 1};
  Process p2 = {.id = 2,
                .state = STATE_NEW,
                .arrival_time = 0,
                .remaining_burst_time = 5,
                .cpu_bursts = p2_cpu,
                .num_bursts = 1};

  Process *procs[] = {&p1, &p2};
  SimulationCore core;

  // Inicializa com custo de troca = 2 ticks
  core_init(&core, procs, 2, &mock_scheduler, 0, 2);

  // Tick 0: P1 e P2 chegam. P1 vai pra CPU e termina (restava 1).
  core_tick(&core);

  // Tick 1: P2 vai pra CPU. Como P2 != P1, dispara a troca (2 ticks).
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      5, p2.remaining_burst_time,
      "CPU deve estar indisponivel (tick 1 da troca)");

  // Tick 2: Troca continua (resta 1 tick de atraso).
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      5, p2.remaining_burst_time,
      "CPU deve estar indisponivel (tick 2 da troca)");

  // Tick 3: Troca finalizada. P2 finalmente executa 1 tick na CPU.
  core_tick(&core);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      4, p2.remaining_burst_time,
      "Apos a troca, P2 deve executar normalmente na CPU");

  // Verifica a contabilizacao global
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, core.total_context_switches,
      "Troca de P1 para P2 conta como 1. Saida do ocioso para P1 nao conta.");

  core_destroy(&core);
}

void test_dispatch_after_idle_does_not_count_context_switch(void) {
  int first_cpu[] = {1}, second_cpu[] = {1};
  Process first = {.id = 1,
                   .state = STATE_NEW,
                   .arrival_time = 0,
                   .cpu_bursts = first_cpu,
                   .num_bursts = 1};
  Process second = {.id = 2,
                    .state = STATE_NEW,
                    .arrival_time = 2,
                    .cpu_bursts = second_cpu,
                    .num_bursts = 1};
  Process *processes[] = {&first, &second};
  SimulationCore core;

  TEST_ASSERT_TRUE(core_init(&core, processes, 2, &mock_scheduler, 0, 2));
  core_tick(&core);
  core_tick(&core);
  core_tick(&core);

  TEST_ASSERT_EQUAL_INT(STATE_FINISHED, second.state);
  TEST_ASSERT_EQUAL_INT(0, core.total_context_switches);
  core_destroy(&core);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_negative_switch_cost_validation);
  RUN_TEST(test_parallel_io_completion);
  RUN_TEST(test_context_switch_cpu_unavailable);
  RUN_TEST(test_dispatch_after_idle_does_not_count_context_switch);
  return UNITY_END();
}
