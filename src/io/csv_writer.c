#include "csv_writer.h"
#include <stdio.h>
#include <string.h>

static bool fail(char *error, size_t error_size, const char *message) {
  if (error != NULL && error_size > 0)
    snprintf(error, error_size, "%s", message);
  return false;
}

static bool text_is_csv_safe(const char *text) {
  return text != NULL && text[0] != '\0' && strchr(text, ',') == NULL &&
         strchr(text, '"') == NULL && strchr(text, '\n') == NULL &&
         strchr(text, '\r') == NULL;
}

bool csv_write_run(const char *path, const RunMetadata *metadata, char *error,
                   size_t error_size) {
  FILE *file;
  int write_result;

  if (path == NULL || path[0] == '\0' || metadata == NULL)
    return fail(error, error_size, "metadados ou caminho CSV invalidos");
  if (!text_is_csv_safe(metadata->algorithm) ||
      !text_is_csv_safe(metadata->scenario))
    return fail(error, error_size, "campo textual nao pode ser gravado no CSV");
  if (metadata->total_processes <= 0 || metadata->total_simulated_time < 0)
    return fail(error, error_size, "metadados numericos invalidos");

  file = fopen(path, "w");
  if (file == NULL)
    return fail(error, error_size, "nao foi possivel criar o arquivo CSV");

  write_result =
      fprintf(file,
              "algorithm,seed,scenario,total_processes,total_simulated_time\n"
              "%s,%u,%s,%d,%d\n",
              metadata->algorithm, metadata->seed, metadata->scenario,
              metadata->total_processes, metadata->total_simulated_time);
  if (write_result < 0) {
    fclose(file);
    return fail(error, error_size, "falha ao gravar o arquivo CSV");
  }
  if (fclose(file) != 0)
    return fail(error, error_size, "falha ao finalizar o arquivo CSV");

  if (error != NULL && error_size > 0)
    error[0] = '\0';
  return true;
}
