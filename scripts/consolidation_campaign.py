"""Load a complete simulation campaign from CSV."""

import csv
from pathlib import Path

if __package__:
    from .consolidation_models import (
        ALGORITHMS,
        CSV_HEADER,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
    )
    from .consolidation_record import parse_identity, parse_record
    from .consolidation_seeds import load_expected_seeds
else:
    from consolidation_models import (
        ALGORITHMS,
        CSV_HEADER,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
    )
    from consolidation_record import parse_identity, parse_record
    from consolidation_seeds import load_expected_seeds


def _validate_completeness(
    observed: set[tuple[str, str, int]], expected_seeds: tuple[int, ...]
) -> None:
    expected = {
        (scenario, algorithm, seed)
        for scenario in SCENARIOS
        for algorithm in ALGORITHMS
        for seed in expected_seeds
    }
    missing = sorted(expected - observed)
    unexpected = sorted(observed - expected)
    if not missing and not unexpected:
        return
    details = []
    if missing:
        plural = "s" if len(missing) != 1 else ""
        details.append(
            f"{len(missing)} combinação ausente{plural}; primeira: {missing[0]}"
        )
    if unexpected:
        plural = "s" if len(unexpected) != 1 else ""
        details.append(
            f"{len(unexpected)} combinação inesperada{plural}; primeira: {unexpected[0]}"
        )
    raise CampaignValidationError("; ".join(details))


def load_campaign(csv_path: Path, seeds_path: Path) -> list[RunRecord]:
    expected_seeds = load_expected_seeds(seeds_path)
    expected_seed_set = set(expected_seeds)
    records = []
    observed = set()
    try:
        csv_file = csv_path.open("r", newline="", encoding="utf-8")
    except OSError as error:
        raise CampaignValidationError(
            f"não foi possível ler o CSV de simulações: {csv_path}"
        ) from error

    with csv_file:
        reader = csv.reader(csv_file)
        header = next(reader, None)
        if header != list(CSV_HEADER):
            raise CampaignValidationError(
                "cabeçalho inválido; esperado: " + ",".join(CSV_HEADER)
            )
        for line_number, row in enumerate(reader, start=2):
            if len(row) != len(CSV_HEADER):
                raise CampaignValidationError(
                    f"linha {line_number}: esperadas {len(CSV_HEADER)} colunas; "
                    f"encontradas {len(row)}"
                )
            values = dict(zip(CSV_HEADER, row))
            key = parse_identity(values, line_number, expected_seed_set)
            if key in observed:
                raise CampaignValidationError(
                    "combinação duplicada: "
                    f"cenário={key[0]}, algoritmo={key[1]}, seed={key[2]}"
                )
            record = parse_record(values, line_number, expected_seed_set, key)
            observed.add(key)
            records.append(record)

    _validate_completeness(observed, expected_seeds)
    return records
