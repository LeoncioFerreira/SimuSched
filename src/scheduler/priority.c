#include <stdlib.h>
#include "scheduler.h"
#include "process.h"
#include "priority.h"

typedef struct Priority_Node {
    Process* p;
    struct Priority_Node* next;
} Priority_Node;

typedef struct Priority_Queue {
    Priority_Node* head;
} Priority_Queue;


int has_preference(Process* p1, Process* p2) {
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

void priority_enqueue_process(struct Scheduler* self, Process* p) {
    Priority_Queue* queue = (Priority_Queue*) self->state;

    Priority_Node* new_node = (Priority_Node*) malloc(sizeof(Priority_Node));
    new_node->p = p;
    new_node->next = NULL;

    // Caso 1: Fila vazia
    if (queue->head == NULL) {
        queue->head = new_node;
        return;
    }

    if (has_preference(p, queue->head->p)) {
        new_node->next = queue->head;
        queue->head = new_node;
        return;
    }

    Priority_Node* current = queue->head;

    while (current->next != NULL && !has_preference(p, current->next->p)) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;
}

Process* priority_get_next_process(struct Scheduler* self) {
    Priority_Queue* queue = (Priority_Queue*) self->state;

    if (queue->head == NULL) {
        return NULL;
    }

    Priority_Node* top_node = queue->head;
    Process* next_process = top_node->p;

    queue->head = queue->head->next;

    free(top_node);

    return next_process;
}

Scheduler* create_priority_scheduler() {
    Scheduler* sched = (Scheduler*) malloc(sizeof(Scheduler));

    Priority_Queue* queue = (Priority_Queue*) malloc(sizeof(Priority_Queue));
    queue->head = NULL;

    sched->state = queue;
    sched->enqueue_process = priority_enqueue_process;
    sched->get_next_process = priority_get_next_process;

    return sched;
}
