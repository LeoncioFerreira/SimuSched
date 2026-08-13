/**
 * Descrição: Declara a política de escalonamento Round Robin, responsável por
 * manter os processos prontos em ordem FIFO pela interface comum Scheduler.
 * Autor: Leôncio Ferreira
 */
#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "scheduler.h"

Scheduler *create_round_robin_scheduler(int capacity);

#endif
