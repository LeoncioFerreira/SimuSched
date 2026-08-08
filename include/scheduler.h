#include <stdio.h>
#include <stdlib.h>
#include "process.h"

typedef struct Scheduler {

    void (*enqueue_process)(struct Scheduler* self, Process* p); //"função para colocar o processo no final do escalonador"

    Process* (*get_next_process)(struct Scheduler* self);

    void* state;

} Scheduler;