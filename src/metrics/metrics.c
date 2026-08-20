#include "metrics.h"

int calculate_turnaround(const Process *process) {
  return process->finish_time - process->arrival_time;
}

double calculate_slowdown(const Process *process) {
  int total_cpu_time = 0;

  for (int i = 0; i < process->num_bursts; i++)
    total_cpu_time += process->cpu_bursts[i];

  return (double)calculate_turnaround(process) / total_cpu_time;
}

double calculate_average_turnaround(Process *const *processes, int count) {
  double total_turnaround = 0.0;

  for (int i = 0; i < count; i++)
    total_turnaround += calculate_turnaround(processes[i]);

  return total_turnaround / count;
}

double calculate_jain_slowdown(Process *const *processes, int count) {
  double slowdown_sum = 0.0;
  double squared_slowdown_sum = 0.0;

  for (int i = 0; i < count; i++) {
    double slowdown = calculate_slowdown(processes[i]);
    slowdown_sum += slowdown;
    squared_slowdown_sum += slowdown * slowdown;
  }

  return (slowdown_sum * slowdown_sum) / (count * squared_slowdown_sum);
}
