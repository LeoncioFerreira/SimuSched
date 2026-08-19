# SimuSched

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml/badge.svg)](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

SimuSched será um simulador de escalonamento de processos para a disciplina de Sistemas Operacionais. O projeto comparará algoritmos clássicos com um algoritmo proposto pela equipe, usando cargas de trabalho determinísticas geradas por seed e métricas quantitativas de desempenho e justiça.

> Status atual: **Projeto Finalizado**. Núcleo de simulação, gerador determinístico, pipeline automatizado e todos os escalonadores (FCFS, Prioridade Não Preemptiva, Round-Robin e o algoritmo próprio **TEJ**) implementados.

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
| **André Wesley** | CLI, Implementação das Métricas (Jain/Turnaround) e Consolidação Estatística (Python) | [@AndreWesley](https://github.com/AndreWesley) |
| **Leôncio Ferreira** | Núcleo do Simulador, Automação (`make run-all`), Round Robin e Redação do Artigo | [@LeoncioFerreira](https://github.com/LeoncioFerreira) |
| **Paulo Gabriel** | Escalonadores Básicos (FCFS/Prioridade), Cenários/Seeds, Geração de Gráficos e Slides | [@LandimPG](https://github.com/LandimPG) |
| **Salomão Rodrigues** | Gerador Determinístico, Modelagem de E/S Paralela e Custo de Troca de Contexto | [@SalomaoRodrigues](https://github.com/SalomaoRodrigues) |

*Nota: O algoritmo próprio (TEJ) foi projetado e desenvolvido em conjunto por toda a equipe.* As responsabilidades individuais listadas acima refletem a divisão de tarefas do Kanban oficial do projeto.

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

### Execução do Pipeline Completo (Simulação, Estatística e Gráficos)

O repositório conta com um pipeline totalmente automatizado. Para rodar as 1.600 simulações C em lote (`run-all`), consolidar as estatísticas (`stats`) e plotar as imagens finais (`figures`) de uma só vez, ative o ambiente virtual e execute o comando principal:

```bash
source venv/bin/activate
make figures
```

O alvo `figures` encadeia automaticamente as dependências, garantindo que os dados brutos e processados sejam recriados sempre que necessário antes de gerar os gráficos na pasta `results/figures/` e `docs/report/figuras/`.

#### Execução Isolada de Etapas
Caso deseje executar ou testar as etapas do pipeline de forma isolada:

- **Simulação Bruta (`make run-all`):** Roda a campanha completa (100 seeds x 4 cenários x 4 algoritmos) e escreve o CSV bruto em `data/raw/simulations.csv`.
- **Consolidação (`make stats`):** Valida os dados brutos e escreve as médias e o IC95% bilateral (Student) em `data/processed/` (Turnaround, Trocas de Contexto e Índice de Jain).
- **Testes Estatísticos (`make test-stats`):** Roda a suíte de testes em Python isoladamente para garantir a precisão matemática da consolidação.

## ⚙️ Pipeline de Integração Contínua (CI)

O GitHub Actions em [`.github/workflows/ci.yml`](.github/workflows/ci.yml) executa em pushes e pull requests para `main` e `develop`.

O pipeline valida a estrutura obrigatória, instala `cppcheck` e `clang-format`, verifica a formatação, executa o linter e roda `make all` e `make test`. Assim, toda alteração precisa manter build e testes automatizados aprovados.

## 📅 Entrega

Conforme o enunciado da disciplina, a entrega está prevista para **19 de agosto de 2026**. O repositório deverá conter o código, instruções de execução, configurações, resultados consolidados e os materiais de apresentação.
