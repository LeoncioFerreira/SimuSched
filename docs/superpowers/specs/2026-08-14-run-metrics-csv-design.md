# Design das métricas e do CSV por execução

## Objetivo

Implementar as métricas exigidas pelo SimuSched e produzir um CSV padronizado com uma linha de resumo por execução. O resumo identifica o algoritmo, o cenário, a seed e a configuração utilizada, preserva o tempo total simulado já exportado e acrescenta os parâmetros e resultados necessários para comparação experimental.

## Escopo

Esta mudança abrange:

- turnaround por processo;
- slowdown por processo;
- turnaround médio por execução;
- índice de Jain sobre os slowdowns de todos os processos;
- leitura do total de trocas do contador central da simulação;
- inclusão de quantum, custo de troca e caminho da configuração no CSV;
- testes unitários das fórmulas e testes do CSV e da integração.

O índice de Jain será armazenado no CSV na escala de 0 a 1. Consumidores que produzam gráficos devem multiplicar o valor por 100 e apresentar a unidade como percentual. O projeto ainda não possui um gerador de gráficos; criar uma nova ferramenta de visualização está fora deste escopo.

## Arquitetura

As responsabilidades permanecem separadas em três unidades:

1. `include/metrics.h` e `src/metrics/metrics.c` expõem funções puras para métricas por processo e agregadas.
2. `src/simulator.c` calcula os agregados após o término da simulação, enquanto o workload e o núcleo ainda estão disponíveis, e monta os metadados da execução.
3. `include/csv_writer.h` e `src/io/csv_writer.c` validam os metadados já calculados e apenas os serializam. O exportador não examina processos e não recalcula trocas de contexto.

## Fórmulas e contratos

`calculate_turnaround(const Process *process)` calcula:

```text
finish_time - arrival_time
```

`calculate_slowdown(const Process *process)` soma exatamente os `num_bursts` elementos de `cpu_bursts` e calcula:

```text
turnaround / total_cpu_time
```

Rajadas de E/S não entram no tempo ideal porque slowdown compara o tempo observado com o tempo de serviço necessário na CPU. A divisão usa aritmética de ponto flutuante para não truncar resultados.

`calculate_average_turnaround(Process *const *processes, int count)` calcula a média aritmética dos turnarounds.

`calculate_jain_slowdown(Process *const *processes, int count)` calcula os slowdowns e aplica:

```text
((sum slowdown) ^ 2) / (count * sum slowdown^2)
```

As funções não devem desreferenciar ponteiros nulos nem dividir por zero. `calculate_turnaround` retorna `-1` para processo nulo. Ponteiro nulo, quantidade não positiva, vetor de bursts ausente, número de bursts não positivo ou soma de CPU não positiva são entradas inválidas e fazem as funções `double` retornarem `-1.0`. A integração usa apenas workloads válidos gerados pelo projeto.

## Fluxo de dados

Após `core_is_finished` tornar-se verdadeiro:

1. `simulator.c` calcula turnaround médio e Jain a partir de `workload`.
2. O total de trocas é copiado diretamente de `core.total_context_switches`.
3. O quantum é copiado de `scenario.quantum`, inclusive para algoritmos que não o usam, pois ele identifica a configuração executada.
4. O custo é copiado de `scenario.config.context_switch_cost`.
5. A identificação da configuração recebe `options->config_path`; o nome lógico continua vindo de `scenario.name`.
6. `csv_write_run` grava o cabeçalho e exatamente uma linha de resumo.

## Contrato do CSV

O CSV preserva os campos existentes e acrescenta os campos exigidos. A ordem padronizada será:

```text
algorithm,seed,scenario,configuration,total_processes,total_simulated_time,quantum,context_switch_cost,average_turnaround,context_switches,jain_slowdown
```

Campos de ponto flutuante serão gravados com precisão decimal suficiente para reproduzir os valores calculados. `jain_slowdown` deve permanecer entre 0 e 1 para workloads válidos.

## Validação e erros

O escritor rejeita caminho ou metadados nulos, campos textuais vazios ou inseguros para o formato CSV, quantidade não positiva de processos, tempos ou contadores negativos, quantum ou custo negativos e Jain fora de 0 a 1. Falhas de abertura, escrita e fechamento continuam sendo informadas pelo buffer de erro existente.

Como o caminho da configuração passa a ser exportado sem mecanismo de escape, ele segue a mesma regra de segurança dos demais campos textuais: vírgula, aspas e quebras de linha são rejeitadas.

## Estratégia de testes

O desenvolvimento seguirá ciclos TDD pequenos:

- turnaround: chegada 3, finalização 13, resultado 10;
- slowdown: chegada 2, finalização 14, bursts de CPU 2 e 4, resultado 2,0;
- Jain: slowdowns manuais escolhidos para produzir um valor conhecido pela fórmula;
- turnaround médio: conjunto pequeno com média calculada manualmente;
- CSV: comparação literal do cabeçalho e da única linha gerada;
- integração: execução dos três algoritmos e verificação dos identificadores e parâmetros no CSV;
- contador: cenário controlado que confirma que o valor exportado é o valor mantido pelo núcleo.

O build de testes definirá `UNITY_INCLUDE_DOUBLE` para que `TEST_ASSERT_DOUBLE_WITHIN` avalie efetivamente os resultados `double`.

## Fora de escopo

- recalcular trocas no módulo de métricas ou no exportador;
- incluir rajadas de E/S no slowdown;
- exportar Jain como número de 0 a 100;
- criar análises estatísticas, intervalos de confiança ou novos tipos de gráfico;
- refatorações não relacionadas nos escalonadores ou no núcleo.
