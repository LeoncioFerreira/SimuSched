"""Generate comparative bar charts from the consolidated experiment tables."""

import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Ordem canônica alinhada com consolidation_models.py
SCENARIOS = ("balanced", "cpu_bound", "io_bound", "unbalanced_priorities")
ALGORITHMS = ("fcfs", "priority", "round-robin", "tej")

# Nomes legíveis para o artigo e apresentação
SCENARIO_LABELS = {
    "balanced": "Equilibrado",
    "cpu_bound": "CPU-Bound",
    "io_bound": "I/O-Bound",
    "unbalanced_priorities": "Prioridades\nDesbalanceadas",
}

ALGORITHM_LABELS = {
    "fcfs": "FCFS",
    "priority": "Prioridade",
    "round-robin": "Round-Robin",
    "tej": "TEJ",
}

# Paleta de cores consistente para cada algoritmo
ALGORITHM_COLORS = {
    "fcfs": "#4C72B0",
    "priority": "#DD8452",
    "round-robin": "#55A868",
    "tej": "#C44E52",
}


def plot_metric(csv_file, title, ylabel, output_file, ylim_top=None):
    """Read a consolidated CSV and generate a grouped bar chart with CI95%."""
    if not os.path.exists(csv_file):
        print(f"Aviso: {csv_file} não encontrado. Pulando geração deste gráfico.")
        return

    df = pd.read_csv(csv_file)

    x = np.arange(len(SCENARIOS))
    width = 0.18

    fig, ax = plt.subplots(figsize=(12, 7))

    for i, algo in enumerate(ALGORITHMS):
        algo_data = df[df['algorithm'] == algo]

        means = []
        yerr_lower = []
        yerr_upper = []

        for sc in SCENARIOS:
            row = algo_data[algo_data['scenario'] == sc]
            if not row.empty:
                mean = row['mean'].values[0]
                ci_lower = row['ci95_lower'].values[0]
                ci_upper = row['ci95_upper'].values[0]
                means.append(mean)
                yerr_lower.append(mean - ci_lower)
                yerr_upper.append(ci_upper - mean)
            else:
                means.append(0)
                yerr_lower.append(0)
                yerr_upper.append(0)

        bar_positions = x - (width * len(ALGORITHMS) / 2) + (i * width) + (width / 2)

        ax.bar(
            bar_positions, means, width,
            label=ALGORITHM_LABELS[algo],
            color=ALGORITHM_COLORS[algo],
            yerr=[yerr_lower, yerr_upper],
            capsize=6, alpha=0.9,
            edgecolor='white', linewidth=0.5,
            error_kw={'elinewidth': 2, 'capthick': 2, 'ecolor': '#333333'},
        )

    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(title, fontsize=14, fontweight='bold', pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels([SCENARIO_LABELS[sc] for sc in SCENARIOS], fontsize=10)

    if ylim_top is not None:
        ax.set_ylim(top=ylim_top)

    # Legenda fora do gráfico, centralizada abaixo
    ax.legend(
        title="Algoritmo", fontsize=10, title_fontsize=10,
        loc='upper center', bbox_to_anchor=(0.5, -0.08),
        ncol=len(ALGORITHMS), frameon=True,
    )

    # Nota de rodapé explicando IC 95%
    fig.text(
        0.5, -0.02,
        'IC 95% = Intervalo de Confiança de 95% (n = 100 sementes)',
        ha='center', fontsize=9, fontstyle='italic', color='#555555',
    )

    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.set_axisbelow(True)

    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Gerado: {output_file}")


def main():
    processed_dir = 'data/processed'
    figures_dir = 'results/figures'

    plots = [
        (
            'average_turnaround.csv',
            'Comparação de Turnaround Médio por Cenário e Algoritmo',
            'Turnaround Médio (unidades de tempo)',
            'turnaround_medio.png',
            None
        ),
        (
            'context_switches.csv',
            'Comparação de Trocas de Contexto por Cenário e Algoritmo',
            'Trocas de Contexto (quantidade)',
            'trocas_contexto.png',
            None
        ),
        (
            'jain_slowdown_percent.csv',
            'Comparação do Índice de Jain por Cenário e Algoritmo',
            'Índice de Jain (%)',
            'jain_slowdown.png',
            100  # Índice de Jain é matematicamente limitado a 100%
        ),
    ]

    for csv_name, title, ylabel, out_name, ylim_top in plots:
        csv_file = os.path.join(processed_dir, csv_name)
        output_file = os.path.join(figures_dir, out_name)
        plot_metric(csv_file, title, ylabel, output_file, ylim_top=ylim_top)

if __name__ == '__main__':
    main()
