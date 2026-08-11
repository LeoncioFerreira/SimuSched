/**
 * Descrição: Função principal do simulador. Responsável por interpretar os
 * argumentos da linha de comando e iniciar a execução do simulador,
 * exibindo mensagens de erro quando necessário.
 * Autor: André Wesley
 */

#include "cli.h"
#include "simulator.h"
#include <stdio.h>

int main(int argc, char **argv) {
  CliOptions options;
  char error[256];

  if (cli_parse(argc, argv, &options, error, sizeof(error)) != CLI_PARSE_OK) {
    fprintf(stderr, "Erro: %s\n", error);
    cli_print_usage(stderr, argc > 0 ? argv[0] : "simulador");
    return 1;
  }
  if (!run_simulator(&options, error, sizeof(error))) {
    fprintf(stderr, "Erro: %s\n", error);
    return 1;
  }
  return 0;
}
