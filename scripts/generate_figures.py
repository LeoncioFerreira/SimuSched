import os
import csv
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['figure.dpi'] = 300

SCENARIOS  = ("balanced", "cpu_bound", "io_bound", "unbalanced_priorities")
ALGORITHMS = ("fcfs", "priority", "round-robin", "tej")

SCENARIO_LABELS   = ["Equilibrado", "CPU-Bound", "I/O-Bound", "Desbal."]
ALGORITHM_LABELS  = ["FCFS", "Prioridade", "Round-Robin", "TEJ"]

ALGORITHM_COLORS = {
    "fcfs":        "#2b5c8f",
    "priority":    "#d95f02",
    "round-robin": "#7570b3",
    "tej":         "#1b9e77",
}

def load_data(csv_file):
    data = {}
    with open(csv_file, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            sc   = row['scenario']
            algo = row['algorithm']
            if sc not in data:
                data[sc] = {}
            mean = float(row['mean'])
            data[sc][algo] = {
                'mean':     mean,
                'err_low':  mean - float(row['ci95_lower']),
                'err_high': float(row['ci95_upper']) - mean,
            }
    return data


def grouped_bar(ax, data, scale=1.0, fmt_fn=None, y_start=0, y_pad=1.40, label_gap_frac=0.03):
    n_s     = len(SCENARIOS)
    n_a     = len(ALGORITHMS)
    group_w = 0.74
    slot_w  = group_w / n_a
    bar_w   = slot_w * 0.65
    offsets = np.linspace(-group_w/2 + slot_w/2, group_w/2 - slot_w/2, n_a)
    x       = np.arange(n_s)

    all_means = [data[sc][a]['mean'] / scale for sc in SCENARIOS for a in ALGORITHMS]
    max_val   = max(all_means)

    for i, algo in enumerate(ALGORITHMS):
        means = np.array([data[sc][algo]['mean'] / scale for sc in SCENARIOS])
        el    = np.array([data[sc][algo]['err_low']  / scale for sc in SCENARIOS])
        eh    = np.array([data[sc][algo]['err_high'] / scale for sc in SCENARIOS])

        bars = ax.bar(
            x + offsets[i], means, width=bar_w,
            color=ALGORITHM_COLORS[algo], edgecolor='black', linewidth=0.6,
            yerr=[el, eh], capsize=1.5,
            error_kw={'elinewidth': 0.8, 'capthick': 0.8, 'ecolor': '#333'}
        )

        for j, bar in enumerate(bars):
            v     = means[j]
            label = fmt_fn(v) if fmt_fn else f"{v:.1f}"
            top = v + eh[j] + (max_val - y_start) * label_gap_frac
            ax.text(
                bar.get_x() + bar.get_width() / 2, top,
                label,
                ha='center', va='bottom',
                fontsize=7.5,           # Larger font
                fontweight='bold',
                color='#111',
                rotation=90,
                bbox=dict(             # White background box for contrast
                    boxstyle='round,pad=0.05',
                    facecolor='white',
                    edgecolor='none',
                    alpha=0.75
                )
            )

    ax.set_xticks(x)
    ax.set_xticklabels(SCENARIO_LABELS, fontsize=8)
    ax.set_ylim(y_start, max_val * y_pad)
    ax.grid(axis='y', linestyle='--', alpha=0.55, linewidth=0.7)
    ax.set_axisbelow(True)
    ax.tick_params(axis='y', labelsize=8)


def make_legend(fig):
    handles = [mpatches.Patch(color=ALGORITHM_COLORS[a], label=lbl,
                              ec='black', lw=0.8)
               for a, lbl in zip(ALGORITHMS, ALGORITHM_LABELS)]
    fig.legend(handles=handles, loc='upper center',
               bbox_to_anchor=(0.5, 0.98), ncol=4,
               fontsize=7.5, frameon=True, edgecolor='grey',
               handlelength=1.2, handleheight=0.9,
               borderaxespad=0.1)


def save_both(fig, out_dir, name):
    os.makedirs(out_dir, exist_ok=True)
    fig.savefig(os.path.join(out_dir, f'{name}.pdf'), format='pdf', bbox_inches='tight')
    fig.savefig(os.path.join(out_dir, f'{name}.png'), format='png', dpi=300, bbox_inches='tight')
    print(f"  ✓ {out_dir}/{name}.pdf")


OUT_DIRS = [
    'docs/report/figuras',
    '/home/leoncio/Desktop/Repositoriso-codigo/artigo/figuras',
    'results/figures',
]

FW = 8.8 / 2.54
FH = 6.5 / 2.54   # Reduced height to fit within 6-page limit

def main():
    proc = 'data/processed'
    t_data = load_data(os.path.join(proc, 'average_turnaround.csv'))
    c_data = load_data(os.path.join(proc, 'context_switches.csv'))
    j_data = load_data(os.path.join(proc, 'jain_slowdown_percent.csv'))

    # ── Turnaround ─────────────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(FW, FH))
    grouped_bar(ax, t_data, scale=1000, fmt_fn=lambda v: f"{v:.1f}k")
    ax.set_ylabel('Turnaround Médio (k ticks)', fontsize=8)
    ax.set_title('Turnaround Médio por Cenário', fontsize=9, fontweight='bold', pad=4)
    make_legend(fig)
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    for d in OUT_DIRS:
        save_both(fig, d, 'turnaround_medio')
    plt.close(fig)

    # ── Context Switches ───────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(FW, FH))
    grouped_bar(ax, c_data, scale=1,
                fmt_fn=lambda v: f"{int(round(v)):,}".replace(',', '.'))
    ax.set_ylabel('Trocas de Contexto', fontsize=8)
    ax.set_title('Trocas de Contexto por Cenário', fontsize=9, fontweight='bold', pad=4)
    make_legend(fig)
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    for d in OUT_DIRS:
        save_both(fig, d, 'trocas_contexto')
    plt.close(fig)

    # ── Jain Fairness ─────────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(FW, FH))
    grouped_bar(ax, j_data, scale=1,
                fmt_fn=lambda v: f"{v:.1f}%",
                y_start=0, y_pad=1.40, label_gap_frac=0.03)
    ax.set_ylabel('Índice de Jain do Slowdown (%)', fontsize=8)
    ax.set_title('Índice de Jain por Cenário', fontsize=9, fontweight='bold', pad=4)
    make_legend(fig)
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    for d in OUT_DIRS:
        save_both(fig, d, 'jain_slowdown')
    plt.close(fig)

    print("Todas as figuras regeneradas!")

if __name__ == '__main__':
    main()
