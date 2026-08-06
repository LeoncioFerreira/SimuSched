#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
  STATE_NEW,
  STATE_READY,
  STATE_RUNNING,
  STATE_BLOCKED,
  STATE_FINISHED,
} ProcessState;

typedef struct {
  int id;
  int arrival_time;
  int finish;
  ProcessState state;

  int *cpu_bursts;
  int *io_bursts;
  int num_bursts;
  int current_burst_index;
  int remaining_burst_time;
} Process;

#endif
