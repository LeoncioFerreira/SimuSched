#include "priority.h"
#include "process.h"
#include "scheduler.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct Priority_Node {
  Process *p;
  struct Priority_Node *next;
} Priority_Node;

typedef struct Priority_Queue {
  Priority_Node *head;
} Priority_Queue;

static int has_preference(const Process *p1, const Process *p2) {
  if (p1->priority < p2->priority) {
    return 1;
  }
  if (p1->priority > p2->priority) {
    return 0;
  }
  if (p1->ready_queue_arrival_time < p2->ready_queue_arrival_time) {
    return 1;
  }
  if (p1->ready_queue_arrival_time > p2->ready_queue_arrival_time) {
    return 0;
  }
  if (p1->id < p2->id) {
    return 1;
  }
  return 0;
}

static bool priority_enqueue_process(struct Scheduler *self, Process *p) {
  Priority_Queue *queue = (Priority_Queue *)self->state;

  Priority_Node *new_node = (Priority_Node *)malloc(sizeof(Priority_Node));
  if (!new_node)
    return false;

  new_node->p = p;
  new_node->next = NULL;

  // Caso 1: Fila vazia
  if (queue->head == NULL) {
    queue->head = new_node;
    return true;
  }

  if (has_preference(p, queue->head->p)) {
    new_node->next = queue->head;
    queue->head = new_node;
    return true;
  }

  Priority_Node *current = queue->head;

  while (current->next != NULL && !has_preference(p, current->next->p)) {
    current = current->next;
  }

  new_node->next = current->next;
  current->next = new_node;
  return true;
}

static Process *priority_get_next_process(struct Scheduler *self,
                                          int current_time) {
  (void)current_time;
  Priority_Queue *queue = (Priority_Queue *)self->state;

  if (queue->head == NULL) {
    return NULL;
  }

  Priority_Node *top_node = queue->head;
  Process *next_process = top_node->p;

  queue->head = queue->head->next;

  free(top_node);

  return next_process;
}

static bool priority_is_empty(struct Scheduler *self) {
  const Priority_Queue *queue = (const Priority_Queue *)self->state;
  return queue->head == NULL;
}

static void priority_destroy(struct Scheduler *self) {
  Priority_Queue *queue = (Priority_Queue *)self->state;
  Priority_Node *current = queue->head;
  while (current != NULL) {
    Priority_Node *temp = current;
    current = current->next;
    free(temp);
  }
  free(queue);
  free(self);
}

Scheduler *create_priority_scheduler() {
  Scheduler *sched = (Scheduler *)malloc(sizeof(Scheduler));
  if (!sched)
    return NULL;

  Priority_Queue *queue = (Priority_Queue *)malloc(sizeof(Priority_Queue));
  if (!queue) {
    free(sched);
    return NULL;
  }
  queue->head = NULL;

  sched->state = queue;
  sched->enqueue_process = priority_enqueue_process;
  sched->get_next_process = priority_get_next_process;
  sched->is_empty = priority_is_empty;
  sched->destroy = priority_destroy;

  return sched;
}
