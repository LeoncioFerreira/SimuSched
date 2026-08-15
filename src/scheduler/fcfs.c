#include "fcfs.h"
#include "circular_queue.h"
#include "process.h"
#include "scheduler.h"
#include <stdlib.h>

static bool fcfs_enqueue_process(struct Scheduler *self, Process *p) {
  CircularQueue *queue = (CircularQueue *)self->state;
  return circular_queue_enqueue(queue, p);
}

static Process *fcfs_get_next_process(struct Scheduler *self) {
  CircularQueue *queue = (CircularQueue *)self->state;
  return circular_queue_dequeue(queue);
}

static bool fcfs_is_empty(struct Scheduler *self) {
  const CircularQueue *queue = (const CircularQueue *)self->state;
  return circular_queue_is_empty(queue);
}

static void fcfs_destroy(struct Scheduler *self) {
  CircularQueue *queue = (CircularQueue *)self->state;
  circular_queue_destroy(queue);
  free(self);
}

Scheduler *create_fcfs_scheduler(int capacity) {
  Scheduler *sched = (Scheduler *)malloc(sizeof(Scheduler));
  if (!sched)
    return NULL;

  CircularQueue *queue = circular_queue_create(capacity);
  if (!queue) {
    free(sched);
    return NULL;
  }

  sched->state = queue;
  sched->enqueue_process = fcfs_enqueue_process;
  sched->get_next_process = fcfs_get_next_process;
  sched->is_empty = fcfs_is_empty;
  sched->destroy = fcfs_destroy;

  return sched;
}
