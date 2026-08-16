# Consolidação estatística das simulações

Este documento registra o método usado para transformar as 1.600 execuções da campanha em tabelas comparáveis por cenário e algoritmo.

## Validação da campanha

O consolidador exige o cabeçalho completo emitido pelo simulador e valida o produto cartesiano formado por quatro cenários, quatro algoritmos e pelas 100 seeds distintas de `configs/seeds.txt`. Cada combinação `(scenario, algorithm, seed)` deve aparecer exatamente uma vez. Cabeçalhos divergentes, linhas malformadas, combinações ausentes, inesperadas ou duplicadas interrompem o processamento antes da publicação das tabelas.

Também são rejeitados valores não numéricos, não finitos ou fora do domínio: turnaround e trocas de contexto devem ser não negativos, e Jain deve estar no intervalo `[0, 1]`.

## Estatísticas e IC95%

Para cada par `(scenario, algorithm)` são consolidadas 100 observações de turnaround médio, trocas de contexto e índice de Jain do slowdown. São calculados média aritmética, desvio padrão amostral com denominador `n - 1`, tamanho da amostra e intervalo de confiança bilateral de 95% para a média.

O intervalo usa a distribuição t de Student:

```text
IC95% = mean ± t(0.975, n - 1) × s / sqrt(n)
```

Na campanha completa, `n = 100`, os graus de liberdade são `99` e o valor crítico é `1.984216952`. O script mantém esse valor tabelado explicitamente e não depende de bibliotecas estatísticas externas.

O índice de Jain é multiplicado por 100 antes da consolidação. Portanto, média, desvio padrão e limites do IC95% de `jain_slowdown_percent.csv` estão expressos em pontos percentuais.

## Tabelas

São produzidos em `data/processed/`:

- `average_turnaround.csv`;
- `context_switches.csv`;
- `jain_slowdown_percent.csv`.

Cada arquivo possui 16 linhas de dados, uma por par de cenário e algoritmo, e o cabeçalho:

```text
scenario,algorithm,mean,sample_standard_deviation,n,ci95_lower,ci95_upper
```

Todas as tabelas temporárias são concluídas antes da substituição dos arquivos finais. Uma falha de validação preserva os resultados consolidados existentes.
