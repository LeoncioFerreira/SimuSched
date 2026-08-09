/**
 * Descrição: É o núcleo da simulação. Este arquivo é responsável por avançar
 * o tempo da simulação a cada ciclo e movimentar os processos corretamente
 * entre seus diferentes estados (novo, pronto, em execução, bloqueado e
 * finalizado). Autor: Leôncio Ferreira
 */
#include "simulation_core.h"
#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

void core_init(SimulationCore *core, Process **processes, int total_processes, Scheduler *scheduler) {
  core->current_time = 0;
  core->completed_processes = 0;
  core->total_processes = total_processes;
  core->running_process = NULL;
  core->incoming_processes = processes;

  core->scheduler = scheduler;
  core->blocked_queue = circular_queue_create(total_processes);
}

bool core_is_finished(SimulationCore *core) {
  return core->completed_processes == core->total_processes;
}

void core_tick(SimulationCore *core) {
  int i;

  // novo -> pronto
  for (i = 0; i < core->total_processes; i++) {
    Process *p = core->incoming_processes[i];
    if (p->state == STATE_NEW && p->arrival_time == core->current_time) {
      p->state = STATE_READY;
      p->current_burst_index = 0;
      p->remaining_burst_time = p->cpu_bursts[0]; // Inicializa o burst
      p->ready_queue_arrival_time = core->current_time;
      if (!scheduler_enqueue_process(core->scheduler, p)) {
        fprintf(stderr, "Erro ao inserir processo %d na fila de prontos\n", p->id);
      }
    }
  }

  // bloqueado -> pronto
  int blocked_count = core->blocked_queue->size;
  for (i = 0; i < blocked_count; i++) {
    Process *p = circular_queue_dequeue(core->blocked_queue);
    p->remaining_burst_time--;

    // Se o tempo de i/o acaba o processo volta a fila de prontos
    if (p->remaining_burst_time <= 0) {
      p->state = STATE_READY;
      p->current_burst_index++;

      p->remaining_burst_time = p->cpu_bursts[p->current_burst_index];
      p->ready_queue_arrival_time = core->current_time;
      if (!scheduler_enqueue_process(core->scheduler, p)) {
        fprintf(stderr, "Erro ao inserir processo %d na fila de prontos\n", p->id);
      }
    } else {
      // Ainda bloqueado, devolve para a fila de bloqueados
      circular_queue_enqueue(core->blocked_queue, p);
    }
  }

  // Em execução
  if (core->running_process == NULL &&
      !scheduler_is_empty(core->scheduler)) {
    core->running_process = scheduler_get_next_process(core->scheduler);
    core->running_process->state = STATE_RUNNING;
  }

  if (core->running_process != NULL) {
    core->running_process->remaining_burst_time--;

    if (core->running_process->remaining_burst_time <= 0) {
      Process *rp = core->running_process;

      // Último burst da cpu
      if (rp->current_burst_index >= rp->num_bursts - 1) {
        // Pronto -> Finalizado
        rp->state = STATE_FINISHED;
        rp->finish_time = core->current_time;

        core->completed_processes++;
      } else {
        rp->state = STATE_BLOCKED;
        rp->remaining_burst_time = rp->io_bursts[rp->current_burst_index];
        circular_queue_enqueue(core->blocked_queue, rp);
      }
      core->running_process = NULL;
    }
  }
  core->current_time++;
}

void core_destroy(SimulationCore *core) {
  if (core->scheduler != NULL) {
    scheduler_destroy(core->scheduler);
  }
  if (core->blocked_queue != NULL) {
    circular_queue_destroy(core->blocked_queue);
  }
}
