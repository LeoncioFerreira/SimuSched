/**
 * Descrição: Estrutura principal do simulador. É aqui que centralizamos
 * o relógio global e as filas que controlam quais processos estão aguardando
 * processamento ou esperando operações de Entrada/Saída.
 * Autor: Leôncio Ferreira
 */
#ifndef SIMULATION_CORE_H
#define SIMULATION_CORE_H

#include "./process.h"
#include "circular_queue.h"
#include "scheduler.h"

typedef struct {
  int current_time;
  int completed_processes;
  int total_processes;

  Process *running_process;

  Scheduler *scheduler;
  CircularQueue *blocked_queue;

  Process **incoming_processes;
} SimulationCore;

void core_init(SimulationCore *core, Process **processes, int total_processes,
               Scheduler *scheduler);
void core_tick(SimulationCore *core);
bool core_is_finished(SimulationCore *core);
void core_destroy(SimulationCore *core);

#endif
