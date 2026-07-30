# SimuSched

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml/badge.svg)](https://github.com/LeoncioFerreira/SimuSched/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

SimuSched será um simulador de escalonamento de processos para a disciplina de Sistemas Operacionais. O projeto comparará algoritmos clássicos com um algoritmo proposto pela equipe, usando cargas de trabalho determinísticas geradas por seed e métricas quantitativas de desempenho e justiça.

> Status atual: repositório organizado. A implementação do simulador ainda não foi iniciada.

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

Quando a implementação estiver disponível, o fluxo reproduzível será:

1. Definir um cenário e suas seeds em `configs/`.
2. Compilar o simulador C.
3. Executar todos os algoritmos com a mesma carga para cada seed.
4. Gravar os resultados individuais em `data/raw/`.
5. Consolidar médias e intervalos de confiança em `data/processed/`.
6. Gerar gráficos em `results/figures/` para o artigo e os slides.

Os comandos exatos de compilação, testes e experimentos serão acrescentados junto com a implementação, por meio de um `Makefile` e scripts documentados.

## 🧪 Qualidade e reprodutibilidade

A futura suíte de testes verificará, no mínimo, a geração idêntica de cargas para a mesma seed, as transições de estado dos processos, as regras de cada escalonador e o cálculo das métricas.

O projeto usa o framework [Unity](https://github.com/ThrowTheSwitch/Unity), versionado em `tests/vendor/`, como no repositório de referência. O teste de sanidade atual pode ser executado com:

```bash
make test
```

Os alvos disponíveis por enquanto são `make all`, `make test`, `make lint`, `make format` e `make clean`.

## ⚙️ Pipeline de Integração Contínua (CI)

O GitHub Actions em [`.github/workflows/ci.yml`](.github/workflows/ci.yml) executa em pushes e pull requests para `main` e `develop`.

Enquanto o simulador ainda não possui fontes C, o pipeline valida a estrutura obrigatória do repositório e a documentação de issues. Quando fontes C forem adicionados, ele também instalará `cppcheck` e `clang-format`, verificará formatação, executará o linter e rodará `make all` e `make test`. Assim, a entrega passa a exigir build e testes automatizados sem permitir que uma implementação parcial silencie essas verificações.

## 📅 Entrega

Conforme o enunciado da disciplina, a entrega está prevista para **19 de agosto de 2026**. O repositório deverá conter o código, instruções de execução, configurações, resultados consolidados e os materiais de apresentação.
