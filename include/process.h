#include <stdio.h>

#include <string.h>


typedef struct Process{
    int id;
    int priority;
    long ready_queue_arrival_time; // Instante em que o processo entrou na fila de prontos (para desempate e lógicado FCFS).
    int cpu_burst; // (ou a quantidade de "tiques" de clock) que o processo precisa para executar sua tarefa na CPU

} Process;


