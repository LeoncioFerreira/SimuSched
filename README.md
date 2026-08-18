# SimuSched

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml/badge.svg)](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

SimuSched será um simulador de escalonamento de processos para a disciplina de Sistemas Operacionais. O projeto comparará algoritmos clássicos com um algoritmo proposto pela equipe, usando cargas de trabalho determinísticas geradas por seed e métricas quantitativas de desempenho e justiça.

> Status atual: núcleo de simulação, escalonadores FCFS e prioridade não preemptiva, gerador determinístico e interface de linha de comando implementados.

## 📄 Objetivo acadêmico

O simulador deverá modelar processos, filas de prontos, execuções de CPU, bloqueios de E/S, preempção e custo de troca de contexto. Os experimentos deverão comparar os algoritmos nos cenários exigidos pelo enunciado, usando as mesmas seeds e a mesma configuração para todos eles.

As métricas principais previstas são:

- Turnaround médio;
- Quantidade de trocas de contexto;
- Índice de Jain aplicado ao slowdown;
- Média e intervalo de confiança de 95% para cada cenário e algoritmo.

## 📄 Artefatos

Os materiais finais do trabalho ficarão separados do código:

- Artigo científico, bibliografia, tabelas e figuras: [`docs/report/`](docs/report/)
- Decisões de arquitetura e modelo da simulação: [`docs/design/`](docs/design/)
- Slides da apresentação: [`docs/slides/`](docs/slides/)
- Dados e gráficos consolidados dos experimentos: [`data/`](data/) e [`results/figures/`](results/figures/)

## 👨‍💻 Equipe

| Integrante | Responsabilidade | GitHub |
| --- | --- | --- |
| A definir | A definir | A definir |

As responsabilidades serão registradas e atualizadas antes da entrega, junto do histórico de commits no GitHub.

## 🏗️ Estrutura do Projeto

```text
.
├── README.md               # Visão geral, regras e guia de reprodução
├── LICENSE                 # Licença do projeto
├── .gitignore              # Artefatos de compilação e dados gerados
├── configs/                # Cenários, parâmetros e listas de seeds
├── data/
│   ├── raw/                # Resultados CSV de execuções individuais (não versionados)
│   └── processed/          # Métricas agregadas e IC95% (não versionados)
├── docs/
│   ├── design/             # Decisões de arquitetura e de experimento
│   ├── report/             # Artigo científico, referências e figuras
│   └── slides/             # Apresentação final
├── include/                # Cabeçalhos públicos do simulador em C
├── results/
│   └── figures/            # Gráficos gerados para relatório e slides
├── scripts/                # Automação de experimentos, estatística e gráficos
├── src/                    # Código-fonte C do simulador
│   ├── core/               # Tempo discreto e estado da simulação
│   ├── process/            # Processos, rajadas e estados
│   ├── scheduler/          # Políticas de escalonamento
│   ├── workload/           # Cargas reproduzíveis por seed
│   ├── metrics/            # Métricas por processo e por execução
│   └── io/                 # Configurações e exportação de resultados
└── tests/                  # Testes unitários e de reprodutibilidade
```

## 📜 Regras de Desenvolvimento

### Idioma e documentação

- Código, nomes de arquivos, variáveis e funções serão escritos em inglês.
- README, documentação de design e relatório serão escritos em português.
- Cada decisão que afete comparabilidade ou reprodutibilidade deverá ser registrada em `docs/design/`.
- Toda execução experimental deverá informar cenário, seed, algoritmo e parâmetros utilizados.

### Git e fluxo de trabalho

- Cada alteração deve ser feita em uma branch descritiva e revisada antes de integrar à `main`.
- Commits devem descrever uma mudança única e indicar claramente o trabalho realizado.
- Arquivos compilados, logs, CSVs intermediários e gráficos temporários não devem ser versionados.
- Resultados finais usados no artigo serão identificados e adicionados conscientemente à documentação.

## 🚀 Fluxo previsto de execução

O fluxo reproduzível adotado pelo projeto é:

1. Definir um cenário e suas seeds em `configs/`.
2. Compilar o simulador C.
3. Executar todos os algoritmos com a mesma carga para cada seed.
4. Gravar os resultados individuais em `data/raw/`.
5. Consolidar médias e intervalos de confiança em `data/processed/`.
6. Gerar gráficos em `results/figures/` para o artigo e os slides.

Os comandos de compilação, testes, execução, consolidação estatística e geração de gráficos estão documentados detalhadamente nas seções abaixo.

## 🧪 Qualidade e reprodutibilidade

A suíte de testes verifica a geração idêntica de cargas para a mesma seed, as transições de estado dos processos, as regras de cada escalonador, a CLI, os cenários e a exportação CSV. Os testes de métricas serão adicionados junto desse módulo.

O projeto usa o framework [Unity](https://github.com/ThrowTheSwitch/Unity), versionado em `tests/vendor/`. Toda a suíte pode ser executada com:

```bash
make test
```

Os alvos disponíveis incluem `make all`, `make test`, `make test-stats`, `make stats`, `make lint`, `make format` e `make clean`.

### Compilação e execução

Compile o simulador e execute toda a suíte de testes com:

```bash
make all
make test
```

O binário aceita algoritmo, seed, arquivo de cenário e arquivo CSV de saída:

```text
./bin/simulador --algorithm <fcfs|priority> --seed <n> --config <arquivo> --output <csv>
```

Exemplos usando a mesma carga para os dois algoritmos:

```bash
./bin/simulador --algorithm fcfs --seed 42 --config configs/small.conf --output data/raw/fcfs-small-42.csv
./bin/simulador --algorithm priority --seed 42 --config configs/small.conf --output data/raw/priority-small-42.csv
```

Os arquivos em `configs/` usam o formato `chave=valor`. O cenário inicial
[`configs/small.conf`](configs/small.conf) demonstra todos os campos obrigatórios:

- `scenario`;
- `total_processes`;
- `min_arrival` e `max_arrival`;
- `min_priority` e `max_priority`;
- `high_priority_ratio`;
- `min_cpu_burst_duration` e `max_cpu_burst_duration`;
- `min_io_burst_duration` e `max_io_burst_duration`;
- `min_cpu_bursts` e `max_cpu_bursts`;
- `quantum`;
- `context_switch_cost`.

As rajadas de CPU e E/S e as prioridades são parametrizadas separadamente. O CSV da campanha contém metadados da execução e as métricas `average_turnaround`, `context_switches` e `jain_slowdown`.

### Consolidação estatística

Depois de gerar as 1.600 execuções, regenere as tabelas consolidadas com:

```bash
make run-all
make stats
```

`make stats` valida o cabeçalho e todas as combinações dos quatro cenários, quatro algoritmos e 100 seeds antes de escrever em `data/processed/`:

- `average_turnaround.csv`;
- `context_switches.csv`;
- `jain_slowdown_percent.csv`.

Cada tabela informa média, desvio padrão amostral, tamanho da amostra e IC95% para cada cenário e algoritmo. O IC95% bilateral usa a distribuição t de Student: `mean ± t(0.975, n - 1) × s / sqrt(n)`. Jain é apresentado em pontos percentuais, após conversão da escala original `[0, 1]` para `[0, 100]`.

Os testes estatísticos, inclusive comparações com valores calculados manualmente, podem ser executados isoladamente com:

```bash
make test-stats
```

### Geração de Gráficos

Com as tabelas consolidadas prontas, as imagens utilizadas no artigo e nos slides podem ser (re)geradas com o comando:

```bash
source venv/bin/activate
make figures
```

Este comando lê os arquivos de `data/processed/` e gera os gráficos de barras comparativos na pasta `results/figures/`, com os algoritmos agrupados por cenário e aplicando os Intervalos de Confiança (IC 95%) em cada métrica. Para rodar este comando, certifique-se de ativar o ambiente virtual (venv) contendo as dependências de plotagem.

## ⚙️ Pipeline de Integração Contínua (CI)

O GitHub Actions em [`.github/workflows/ci.yml`](.github/workflows/ci.yml) executa em pushes e pull requests para `main` e `develop`.

O pipeline valida a estrutura obrigatória, instala `cppcheck` e `clang-format`, verifica a formatação, executa o linter e roda `make all` e `make test`. Assim, toda alteração precisa manter build e testes automatizados aprovados.

## 📅 Entrega

Conforme o enunciado da disciplina, a entrega está prevista para **19 de agosto de 2026**. O repositório deverá conter o código, instruções de execução, configurações, resultados consolidados e os materiais de apresentação.
