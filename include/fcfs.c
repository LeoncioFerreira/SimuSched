#include <stdlib.h>
#include "scheduler.h"
#include "process.h"

typedef struct FCFS_Node {
    Process *p;
    struct FCSF_Node* next;

} FCFS_Node;

typedef struct FCFS_Queue {
    FCFS_Node* head;
    FCFS_Node* tail;
} FCFS_Queue;

void fcfs_enqueue_process(struct Scheduler* self, Process* p) {
    FCFS_Queue* queue = (FCFS_Queue*) self->state;

    FCFS_Node* new_node = (FCFS_Node*) malloc(sizeof(FCFS_Node));
    new_node ->p = p;
    new_node->next = NULL;

    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
}

Process* fcfs_get_next_process(struct Scheduler* self) {

    FCFS_Queue* queue = (FCFS_Queue*) self->state;

    if (queue ->head == NULL) {
        return NULL;
    }

    FCFS_Node* first_node =  queue->head;
    Process* next_process = first_node ->p;

    queue ->head = queue->head ->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    free(first_node);

    return next_process;
}