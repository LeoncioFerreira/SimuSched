# Cargas de Trabalho e Cenários (Workloads)

Este documento registra as decisões de projeto referentes à geração das cargas de trabalho para as simulações do SimuSched.

## Distribuição de Tempos de Chegada (Arrival Times)

Para garantir a justiça e a reprodutibilidade na comparação entre os algoritmos de escalonamento, **todos os cenários adotam o mesmo modelo matemático de chegada de processos**.

*   **Modelo Estatístico:** Distribuição Uniforme Discreta.
*   **Implementação:** A função `rand_range` no gerador (`workload.c`/`generator.c`) utiliza um gerador congruencial linear (LCG) acoplado a uma semente determinística (*seed*) para sortear um valor inteiro entre os limites definidos.
*   **Limites Padronizados:** Em todos os quatro cenários exigidos, os processos são distribuídos uniformemente ao longo do tempo com os parâmetros `min_arrival = 0` e `max_arrival = 5000`. 

**Motivo:** Isso garante que a pressão de novos processos (a taxa de nascimento) na fila de "Prontos" (Ready Queue) seja idêntica e consistente. Se mudássemos a distribuição de chegada em um cenário específico, os resultados seriam enviesados e não saberíamos se um algoritmo se saiu melhor por causa de seu código ou porque os processos chegaram de forma mais amigável.

## Modelagem das Prioridades

A distribuição das prioridades é dividida em dois extremos dependendo do arquivo de configuração:

*   **Cenários normais (Equilibrado, CPU-bound, I/O-bound):** A probabilidade de um processo nascer com a prioridade máxima é de **50%** (`high_priority_ratio = 0.5`). É um sorteio equilibrado.
*   **Cenário de Prioridades Desbalanceadas:** A probabilidade é de **85%** para a alta prioridade (`high_priority_ratio = 0.85`). Isso cria um ambiente estressante artificial propício à inanição (*starvation*), com o objetivo de punir algoritmos de escalonamento que não possuam mecanismos de Envelhecimento (*Aging*).
