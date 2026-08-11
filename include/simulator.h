#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "cli.h"
#include <stdbool.h>
#include <stddef.h>

bool run_simulator(const CliOptions *options, char *error, size_t error_size);

#endif
