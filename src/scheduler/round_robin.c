/**
 * Descrição: Implementa a fila de prontos da política Round Robin. A política
 * apenas ordena os processos; o núcleo da simulação controla quantum,
 * preempção e transições de estado.
 * Autor: Leôncio Ferreira
 */
#include "round_robin.h"
#include "circular_queue.h"
#include <stdbool.h>
#include <stdlib.h>

static bool round_robin_enqueue(Scheduler *self, Process *process) {
  CircularQueue *queue = self->state;
  return circular_queue_enqueue(queue, process);
}

static Process *round_robin_get_next(Scheduler *self) {
  CircularQueue *queue = self->state;
  return circular_queue_dequeue(queue);
}

static bool round_robin_is_empty(Scheduler *self) {
  CircularQueue *queue = self->state;
  return circular_queue_is_empty(queue);
}

static void round_robin_destroy(Scheduler *self) {
  circular_queue_destroy(self->state);
  free(self);
}

Scheduler *create_round_robin_scheduler(int capacity) {
  Scheduler *scheduler = malloc(sizeof(Scheduler));
  if (scheduler == NULL)
    return NULL;

  CircularQueue *queue = circular_queue_create(capacity);
  if (queue == NULL || queue->data == NULL) {
    circular_queue_destroy(queue);
    free(scheduler);
    return NULL;
  }

  scheduler->state = queue;
  scheduler->enqueue_process = round_robin_enqueue;
  scheduler->get_next_process = round_robin_get_next;
  scheduler->is_empty = round_robin_is_empty;
  scheduler->destroy = round_robin_destroy;

  return scheduler;
}
