#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdio.h>

typedef struct {
  const char *algorithm;
  unsigned int seed;
  const char *config_path;
  const char *output_path;
} CliOptions;

typedef enum {
  CLI_PARSE_OK = 0,
  CLI_PARSE_ERROR = 1,
} CliParseResult;

CliParseResult cli_parse(int argc, char **argv, CliOptions *options,
                         char *error, size_t error_size);
void cli_print_usage(FILE *stream, const char *program_name);

#endif
