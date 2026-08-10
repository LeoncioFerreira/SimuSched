#include "scheduler.h"
#include <stddef.h>

bool scheduler_enqueue_process(Scheduler *scheduler, Process *process) {
  if (!scheduler || !scheduler->enqueue_process)
    return false;
  return scheduler->enqueue_process(scheduler, process);
}

Process *scheduler_get_next_process(Scheduler *scheduler) {
  if (!scheduler || !scheduler->get_next_process)
    return NULL;
  return scheduler->get_next_process(scheduler);
}

bool scheduler_is_empty(Scheduler *scheduler) {
  if (!scheduler || !scheduler->is_empty)
    return true;
  return scheduler->is_empty(scheduler);
}

void scheduler_destroy(Scheduler *scheduler) {
  if (!scheduler)
    return;
  if (scheduler->destroy) {
    scheduler->destroy(scheduler);
  }
}
