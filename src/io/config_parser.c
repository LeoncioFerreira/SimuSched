#include "config_parser.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  KEY_SCENARIO = 1U << 0,
  KEY_TOTAL_PROCESSES = 1U << 1,
  KEY_MIN_ARRIVAL = 1U << 2,
  KEY_MAX_ARRIVAL = 1U << 3,
  KEY_MIN_PRIORITY = 1U << 4,
  KEY_MAX_PRIORITY = 1U << 5,
  KEY_HIGH_PRIORITY_RATIO = 1U << 6,
  KEY_MIN_CPU_BURST = 1U << 7,
  KEY_MAX_CPU_BURST = 1U << 8,
  KEY_MIN_IO_BURST = 1U << 9,
  KEY_MAX_IO_BURST = 1U << 10,
  KEY_MIN_CPU_BURSTS = 1U << 11,
  KEY_MAX_CPU_BURSTS = 1U << 12,
  KEY_QUANTUM = 1U << 13,
  KEY_CONTEXT_SWITCH_COST = 1U << 14,
  ALL_KEYS = (1U << 15) - 1U,
};

static ConfigParseResult fail(char *error, size_t error_size,
                              const char *message) {
  if (error != NULL && error_size > 0)
    snprintf(error, error_size, "%s", message);
  return CONFIG_PARSE_ERROR;
}

static char *trim(char *text) {
  char *end;
  while (isspace((unsigned char)*text))
    text++;
  if (*text == '\0')
    return text;
  end = text + strlen(text) - 1;
  while (end > text && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';
  return text;
}

static int parse_int(const char *text, int *value) {
  char *end = NULL;
  long parsed;
  errno = 0;
  parsed = strtol(text, &end, 10);
  if (errno == ERANGE || end == text || *end != '\0' || parsed < INT_MIN ||
      parsed > INT_MAX)
    return 0;
  *value = (int)parsed;
  return 1;
}

static int parse_double(const char *text, double *value) {
  char *end = NULL;
  double parsed;
  errno = 0;
  parsed = strtod(text, &end);
  if (errno == ERANGE || end == text || *end != '\0' || parsed < 0.0 ||
      parsed > 1.0)
    return 0;
  *value = parsed;
  return 1;
}

static int config_is_valid(const ScenarioConfig *config) {
  return config->total_processes > 0 && config->min_arrival >= 0 &&
         config->min_arrival <= config->max_arrival &&
         config->min_priority >= 0 &&
         config->min_priority <= config->max_priority &&
         config->min_cpu_burst_duration > 0 &&
         config->min_cpu_burst_duration <= config->max_cpu_burst_duration &&
         config->min_io_burst_duration >= 0 &&
         config->min_io_burst_duration <= config->max_io_burst_duration &&
         config->high_priority_ratio >= 0.0 &&
         config->high_priority_ratio <= 1.0 && config->min_cpu_bursts > 0 &&
         config->min_cpu_bursts <= config->max_cpu_bursts &&
         config->context_switch_cost >= 0;
}

static unsigned int key_flag(const char *key, int **target,
                             Scenario *scenario) {
  ScenarioConfig *config = &scenario->config;
  *target = NULL;
  if (strcmp(key, "scenario") == 0)
    return KEY_SCENARIO;
  if (strcmp(key, "total_processes") == 0) {
    *target = &config->total_processes;
    return KEY_TOTAL_PROCESSES;
  }
  if (strcmp(key, "min_arrival") == 0) {
    *target = &config->min_arrival;
    return KEY_MIN_ARRIVAL;
  }
  if (strcmp(key, "max_arrival") == 0) {
    *target = &config->max_arrival;
    return KEY_MAX_ARRIVAL;
  }
  if (strcmp(key, "min_priority") == 0) {
    *target = &config->min_priority;
    return KEY_MIN_PRIORITY;
  }
  if (strcmp(key, "max_priority") == 0) {
    *target = &config->max_priority;
    return KEY_MAX_PRIORITY;
  }
  if (strcmp(key, "high_priority_ratio") == 0) {
    return KEY_HIGH_PRIORITY_RATIO;
  }
  if (strcmp(key, "min_cpu_burst_duration") == 0) {
    *target = &config->min_cpu_burst_duration;
    return KEY_MIN_CPU_BURST;
  }
  if (strcmp(key, "max_cpu_burst_duration") == 0) {
    *target = &config->max_cpu_burst_duration;
    return KEY_MAX_CPU_BURST;
  }
  if (strcmp(key, "min_io_burst_duration") == 0) {
    *target = &config->min_io_burst_duration;
    return KEY_MIN_IO_BURST;
  }
  if (strcmp(key, "max_io_burst_duration") == 0) {
    *target = &config->max_io_burst_duration;
    return KEY_MAX_IO_BURST;
  }
  if (strcmp(key, "min_cpu_bursts") == 0) {
    *target = &config->min_cpu_bursts;
    return KEY_MIN_CPU_BURSTS;
  }
  if (strcmp(key, "max_cpu_bursts") == 0) {
    *target = &config->max_cpu_bursts;
    return KEY_MAX_CPU_BURSTS;
  }
  if (strcmp(key, "quantum") == 0) {
    *target = &scenario->quantum;
    return KEY_QUANTUM;
  }
  if (strcmp(key, "context_switch_cost") == 0) {
    *target = &config->context_switch_cost;
    return KEY_CONTEXT_SWITCH_COST;
  }
  return 0;
}

ConfigParseResult config_parse_file(const char *path, Scenario *scenario,
                                    char *error, size_t error_size) {
  FILE *file;
  char line[512];
  unsigned int seen = 0;

  if (path == NULL || scenario == NULL)
    return fail(error, error_size, "argumentos internos invalidos");
  file = fopen(path, "r");
  if (file == NULL)
    return fail(error, error_size, "nao foi possivel abrir a configuracao");

  memset(scenario, 0, sizeof(*scenario));
  while (fgets(line, sizeof(line), file) != NULL) {
    char *content = trim(line);
    char *separator;
    char *key;
    char *value;
    int *target;
    unsigned int flag;

    if (*content == '\0')
      continue;
    separator = strchr(content, '=');
    if (separator == NULL || strchr(separator + 1, '=') != NULL) {
      fclose(file);
      return fail(error, error_size, "linha de configuracao malformada");
    }
    *separator = '\0';
    key = trim(content);
    value = trim(separator + 1);
    if (*key == '\0' || *value == '\0') {
      fclose(file);
      return fail(error, error_size, "chave ou valor vazio");
    }

    flag = key_flag(key, &target, scenario);
    if (flag == 0) {
      fclose(file);
      return fail(error, error_size, "chave de configuracao desconhecida");
    }
    if ((seen & flag) != 0) {
      fclose(file);
      return fail(error, error_size, "chave de configuracao duplicada");
    }

    if (flag == KEY_SCENARIO) {
      if (strlen(value) >= sizeof(scenario->name)) {
        fclose(file);
        return fail(error, error_size, "nome do cenario muito longo");
      }
      strcpy(scenario->name, value);
    } else if (flag == KEY_HIGH_PRIORITY_RATIO) {
      if (!parse_double(value, &scenario->config.high_priority_ratio)) {
        fclose(file);
        return fail(error, error_size, "valor decimal invalido para ratio");
      }
    } else if (!parse_int(value, target)) {
      fclose(file);
      return fail(error, error_size, "valor inteiro invalido");
    }
    seen |= flag;
  }

  if (ferror(file)) {
    fclose(file);
    return fail(error, error_size, "erro ao ler a configuracao");
  }
  fclose(file);
  if (seen != ALL_KEYS)
    return fail(error, error_size, "chaves obrigatorias ausentes");
  if (!config_is_valid(&scenario->config) || scenario->quantum <= 0)
    return fail(error, error_size, "intervalos de configuracao invalidos");

  if (error != NULL && error_size > 0)
    error[0] = '\0';
  return CONFIG_PARSE_OK;
}
