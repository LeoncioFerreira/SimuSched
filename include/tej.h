/**
 * Descrição: Declara a política não preemptiva Triagem com Espera Justa
 * (TEJ), que combina prioridade estática e resgate por limite de espera.
 * Autor: Leôncio Ferreira
 */
#ifndef TEJ_H
#define TEJ_H

#include "scheduler.h"

Scheduler *create_tej_scheduler(int capacity, int minimum_priority,
                                int rescue_interval);

#endif
