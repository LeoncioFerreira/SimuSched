/**
 * Descrição: Implementação das operações da Fila Circular. A lógica principal
 * é interligar o fim do array com o seu início, criando um ciclo. Isso garante
 * que a fila possa ser reutilizada infinitamente se houver espaços disponíveis.
 * Autor: Leôncio Ferreira
 */
#include "circular_queue.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

CircularQueue *circular_queue_create(int capacity) {
  CircularQueue *q = (CircularQueue *)malloc(sizeof(CircularQueue));
  if (q != NULL) {
    q->capacity = capacity;
    q->first_element = 0;
    q->last_element = -1;
    q->size = 0;

    // Aloca o array com espaço para capacidade
    q->data = (Process **)malloc(capacity * sizeof(Process *));
  }
  return q;
}

bool circular_queue_is_empty(const CircularQueue *q) { return q->size == 0; }

bool circular_queue_is_full(const CircularQueue *q) {
  return q->size == q->capacity;
}

// Inseri na fila (enqueue)
bool circular_queue_enqueue(CircularQueue *q, Process *p) {
  if (circular_queue_is_full(q)) {
    return false;
  }
  // Se passar da capacidade vira uma fila circular
  q->last_element = (q->last_element + 1) % q->capacity;

  q->data[q->last_element] = p;
  q->size++;
  return true;
}

Process *circular_queue_dequeue(CircularQueue *q) {
  if (circular_queue_is_empty(q)) {
    return NULL;
  }

  Process *p = q->data[q->first_element];

  q->first_element = (q->first_element + 1) % q->capacity;

  q->size--;
  return p;
}

void circular_queue_destroy(CircularQueue *q) {
  if (q != NULL) {
    if (q->data != NULL) {
      free(q->data);
    }
    free(q);
  }
}
