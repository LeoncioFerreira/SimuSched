#ifndef METRICS_H
#define METRICS_H
#include "process.h"

int calculate_turnaround(const Process *process);

double calculate_slowdown(const Process *process);
double calculate_average_turnaround(Process *const *processes, int count);
double calculate_jain_slowdown(Process *const *processes, int count);

#endif
