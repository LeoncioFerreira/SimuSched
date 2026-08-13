#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "workload.h"
#include <stddef.h>

#define SCENARIO_NAME_SIZE 64

typedef struct {
  char name[SCENARIO_NAME_SIZE];
  ScenarioConfig config;
  int quantum;
} Scenario;

typedef enum {
  CONFIG_PARSE_OK = 0,
  CONFIG_PARSE_ERROR = 1,
} ConfigParseResult;

ConfigParseResult config_parse_file(const char *path, Scenario *scenario,
                                    char *error, size_t error_size);

#endif
