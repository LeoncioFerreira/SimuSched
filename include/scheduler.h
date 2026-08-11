#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <stdbool.h>

typedef struct Scheduler {
  void *state;

  bool (*enqueue_process)(struct Scheduler *self, Process *p);
  Process *(*get_next_process)(struct Scheduler *self);
  bool (*is_empty)(struct Scheduler *self);
  void (*destroy)(struct Scheduler *self);
} Scheduler;

bool scheduler_enqueue_process(Scheduler *scheduler, Process *process);
Process *scheduler_get_next_process(Scheduler *scheduler);
bool scheduler_is_empty(Scheduler *scheduler);
void scheduler_destroy(Scheduler *scheduler);

#endif
