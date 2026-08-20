"""Validate and statistically consolidate SimuSched experiment results."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys
from typing import Sequence

if __package__:
    from .consolidation_campaign import load_campaign
    from .consolidation_models import (
        ALGORITHMS,
        CSV_HEADER,
        METRICS,
        OUTPUT_HEADER,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
        Summary,
    )
    from .consolidation_output import write_tables_atomically
    from .consolidation_seeds import load_expected_seeds
    from .consolidation_statistics import (
        aggregate_campaign,
        student_t_critical_95,
        summarize,
    )
else:
    from consolidation_campaign import load_campaign
    from consolidation_models import (
        ALGORITHMS,
        CSV_HEADER,
        METRICS,
        OUTPUT_HEADER,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
        Summary,
    )
    from consolidation_output import write_tables_atomically
    from consolidation_seeds import load_expected_seeds
    from consolidation_statistics import (
        aggregate_campaign,
        student_t_critical_95,
        summarize,
    )

__all__ = [
    "ALGORITHMS",
    "CSV_HEADER",
    "CampaignValidationError",
    "METRICS",
    "OUTPUT_HEADER",
    "RunRecord",
    "SCENARIOS",
    "Summary",
    "aggregate_campaign",
    "load_campaign",
    "load_expected_seeds",
    "main",
    "student_t_critical_95",
    "summarize",
    "write_tables_atomically",
]


def _build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Consolida estatísticas da campanha do SimuSched."
    )
    parser.add_argument("--input", type=Path, default=Path("data/raw/simulations.csv"))
    parser.add_argument("--seeds", type=Path, default=Path("configs/seeds.txt"))
    parser.add_argument("--output-dir", type=Path, default=Path("data/processed"))
    return parser


def main(arguments: Sequence[str] | None = None) -> int:
    options = _build_argument_parser().parse_args(arguments)
    try:
        records = load_campaign(options.input, options.seeds)
        tables = aggregate_campaign(records)
        write_tables_atomically(tables, options.output_dir)
    except (CampaignValidationError, OSError, ValueError) as error:
        print(f"Erro: {error}", file=sys.stderr)
        return 1

    print(
        f"Campanha consolidada: {len(records)} execuções; "
        f"tabelas em {options.output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
