/**
 * Descrição: Define as funções e a estrutura da Fila Circular.
 * Essa estrutura organiza a ordem dos processos de forma contínua,
 * reaproveitando os espaços vazios do array sem precisar alocar e liberar
 * memória a cada uso. Autor: Leôncio Ferreira
 */
#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include "process.h"
#include <stdbool.h>

typedef struct {
  Process **data;
  int capacity;
  int first_element;
  int last_element;
  int size;
} CircularQueue;

// Estabelece contrato das funções
CircularQueue *circular_queue_create(int capacity);
bool circular_queue_enqueue(CircularQueue *q, Process *p);
Process *circular_queue_dequeue(CircularQueue *q);
bool circular_queue_is_empty(const CircularQueue *q);
bool circular_queue_is_full(const CircularQueue *q);
void circular_queue_destroy(CircularQueue *q);

#endif
