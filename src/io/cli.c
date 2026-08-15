#include "cli.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
  OPTION_ALGORITHM = 1U << 0,
  OPTION_SEED = 1U << 1,
  OPTION_CONFIG = 1U << 2,
  OPTION_OUTPUT = 1U << 3,
  ALL_OPTIONS = OPTION_ALGORITHM | OPTION_SEED | OPTION_CONFIG | OPTION_OUTPUT,
};

static CliParseResult fail(char *error, size_t error_size,
                           const char *message) {
  if (error != NULL && error_size > 0) {
    snprintf(error, error_size, "%s", message);
  }
  return CLI_PARSE_ERROR;
}

static int parse_seed(const char *value, unsigned int *seed) {
  char *end = NULL;
  unsigned long parsed;

  if (value == NULL || value[0] == '\0' || value[0] == '-')
    return 0;

  errno = 0;
  parsed = strtoul(value, &end, 10);
  if (errno == ERANGE || end == value || *end != '\0' || parsed > UINT_MAX)
    return 0;

  *seed = (unsigned int)parsed;
  return 1;
}

CliParseResult cli_parse(int argc, char **argv, CliOptions *options,
                         char *error, size_t error_size) {
  unsigned int seen = 0;

  if (options == NULL || argv == NULL)
    return fail(error, error_size, "argumentos internos invalidos");

  memset(options, 0, sizeof(*options));
  if (argc < 2 || (argc - 1) % 2 != 0)
    return fail(error, error_size, "todas as opcoes exigem um valor");

  for (int i = 1; i < argc; i += 2) {
    const char *name = argv[i];
    const char *value = argv[i + 1];
    unsigned int flag;

    if (strcmp(name, "--algorithm") == 0) {
      flag = OPTION_ALGORITHM;
      if (strcmp(value, "fcfs") != 0 && strcmp(value, "priority") != 0 &&
          strcmp(value, "round-robin") != 0 && strcmp(value, "tej") != 0)
        return fail(error, error_size,
                    "algoritmo deve ser fcfs, priority, round-robin ou tej");
      options->algorithm = value;
    } else if (strcmp(name, "--seed") == 0) {
      flag = OPTION_SEED;
      if (!parse_seed(value, &options->seed))
        return fail(error, error_size, "seed deve ser um inteiro sem sinal");
    } else if (strcmp(name, "--config") == 0) {
      flag = OPTION_CONFIG;
      if (value[0] == '\0')
        return fail(error, error_size, "arquivo de configuracao vazio");
      options->config_path = value;
    } else if (strcmp(name, "--output") == 0) {
      flag = OPTION_OUTPUT;
      if (value[0] == '\0')
        return fail(error, error_size, "arquivo CSV vazio");
      options->output_path = value;
    } else {
      return fail(error, error_size, "opcao desconhecida");
    }

    if ((seen & flag) != 0)
      return fail(error, error_size, "opcao duplicada");
    seen |= flag;
  }

  if (seen != ALL_OPTIONS)
    return fail(error, error_size, "opcoes obrigatorias ausentes");

  if (error != NULL && error_size > 0)
    error[0] = '\0';
  return CLI_PARSE_OK;
}

void cli_print_usage(FILE *stream, const char *program_name) {
  const char *name = program_name == NULL ? "simulador" : program_name;
  fprintf(stream,
          "Uso: %s --algorithm <fcfs|priority|round-robin|tej> --seed <n> "
          "--config <arquivo> "
          "--output <csv>\n",
          name);
}
