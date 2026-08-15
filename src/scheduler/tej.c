/**
 * Descrição: Implementa a seleção da política Triagem com Espera Justa
 * (TEJ), incluindo prioridade normal, prazo de resgate e desempates
 * determinísticos.
 * Autor: Leôncio Ferreira
 */
#include "tej.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  Process **items;
  int capacity;
  int count;
  int minimum_priority;
  int rescue_interval;
} TejQueue;

static long long rescue_deadline(const TejQueue *queue,
                                 const Process *process) {
  long long levels = (long long)process->priority - queue->minimum_priority + 1;
  long long wait_limit = levels * queue->rescue_interval;

  if (process->ready_queue_arrival_time > LLONG_MAX - wait_limit)
    return LLONG_MAX;
  return process->ready_queue_arrival_time + wait_limit;
}

static bool normal_precedes(const Process *candidate, const Process *selected) {
  if (candidate->priority != selected->priority)
    return candidate->priority < selected->priority;
  if (candidate->ready_queue_arrival_time != selected->ready_queue_arrival_time)
    return candidate->ready_queue_arrival_time <
           selected->ready_queue_arrival_time;
  return candidate->id < selected->id;
}

static bool rescued_precedes(const TejQueue *queue, const Process *candidate,
                             const Process *selected) {
  long long candidate_deadline = rescue_deadline(queue, candidate);
  long long selected_deadline = rescue_deadline(queue, selected);

  if (candidate_deadline != selected_deadline)
    return candidate_deadline < selected_deadline;
  if (candidate->ready_queue_arrival_time != selected->ready_queue_arrival_time)
    return candidate->ready_queue_arrival_time <
           selected->ready_queue_arrival_time;
  return candidate->id < selected->id;
}

static bool tej_enqueue(Scheduler *self, Process *process) {
  TejQueue *queue = self->state;

  if (process == NULL || process->priority < queue->minimum_priority ||
      queue->count >= queue->capacity)
    return false;
  queue->items[queue->count++] = process;
  return true;
}

static Process *tej_get_next(Scheduler *self, int current_time) {
  TejQueue *queue = self->state;
  int selected_index = -1;
  bool selected_is_rescued = false;

  for (int i = 0; i < queue->count; i++) {
    const Process *candidate = queue->items[i];
    bool candidate_is_rescued =
        (long long)current_time >= rescue_deadline(queue, candidate);

    if (selected_index < 0 || (candidate_is_rescued && !selected_is_rescued) ||
        (candidate_is_rescued && selected_is_rescued &&
         rescued_precedes(queue, candidate, queue->items[selected_index])) ||
        (!candidate_is_rescued && !selected_is_rescued &&
         normal_precedes(candidate, queue->items[selected_index]))) {
      selected_index = i;
      selected_is_rescued = candidate_is_rescued;
    }
  }

  if (selected_index < 0)
    return NULL;

  Process *selected = queue->items[selected_index];
  queue->count--;
  queue->items[selected_index] = queue->items[queue->count];
  return selected;
}

static bool tej_is_empty(Scheduler *self) {
  const TejQueue *queue = self->state;
  return queue->count == 0;
}

static void tej_destroy(Scheduler *self) {
  TejQueue *queue = self->state;
  free(queue->items);
  free(queue);
  free(self);
}

Scheduler *create_tej_scheduler(int capacity, int minimum_priority,
                                int rescue_interval) {
  if (capacity <= 0 || minimum_priority < 0 || rescue_interval <= 0 ||
      (size_t)capacity > SIZE_MAX / sizeof(Process *))
    return NULL;

  Scheduler *scheduler = malloc(sizeof(*scheduler));
  TejQueue *queue = malloc(sizeof(*queue));
  if (scheduler == NULL || queue == NULL) {
    free(scheduler);
    free(queue);
    return NULL;
  }

  queue->items = malloc((size_t)capacity * sizeof(*queue->items));
  if (queue->items == NULL) {
    free(queue);
    free(scheduler);
    return NULL;
  }

  queue->capacity = capacity;
  queue->count = 0;
  queue->minimum_priority = minimum_priority;
  queue->rescue_interval = rescue_interval;
  scheduler->state = queue;
  scheduler->enqueue_process = tej_enqueue;
  scheduler->get_next_process = tej_get_next;
  scheduler->is_empty = tej_is_empty;
  scheduler->destroy = tej_destroy;
  return scheduler;
}
