"""Parse and validate one simulation campaign record."""

import math
from typing import Mapping, Set

if __package__:
    from .consolidation_models import (
        ALGORITHMS,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
    )
else:
    from consolidation_models import (
        ALGORITHMS,
        SCENARIOS,
        CampaignValidationError,
        RunRecord,
    )


def _parse_int(value: str, field: str, line_number: int) -> int:
    try:
        return int(value)
    except ValueError as error:
        raise CampaignValidationError(
            f"linha {line_number}: {field} deve ser um inteiro"
        ) from error


def _parse_float(value: str, field: str, line_number: int) -> float:
    try:
        parsed = float(value)
    except ValueError as error:
        raise CampaignValidationError(
            f"linha {line_number}: {field} deve ser numérico"
        ) from error
    if not math.isfinite(parsed):
        raise CampaignValidationError(
            f"linha {line_number}: {field} deve ser finito"
        )
    return parsed


def parse_identity(
    values: Mapping[str, str], line_number: int, expected_seeds: Set[int]
) -> tuple[str, str, int]:
    scenario = values["scenario"]
    algorithm = values["algorithm"]
    if scenario not in SCENARIOS:
        raise CampaignValidationError(
            f"linha {line_number}: cenário inesperado: {scenario!r}"
        )
    if algorithm not in ALGORITHMS:
        raise CampaignValidationError(
            f"linha {line_number}: algoritmo inesperado: {algorithm!r}"
        )

    seed = _parse_int(values["seed"], "seed", line_number)
    if seed not in expected_seeds:
        raise CampaignValidationError(f"linha {line_number}: seed inesperada: {seed}")
    return scenario, algorithm, seed


def parse_record(
    values: Mapping[str, str], line_number: int, expected_seeds: Set[int],
    identity: tuple[str, str, int] | None = None,
) -> RunRecord:
    scenario, algorithm, seed = identity or parse_identity(
        values, line_number, expected_seeds
    )
    total_processes = _parse_int(values["total_processes"], "total_processes", line_number)
    total_time = _parse_int(values["total_simulated_time"], "total_simulated_time", line_number)
    quantum = _parse_int(values["quantum"], "quantum", line_number)
    switch_cost = _parse_int(values["context_switch_cost"], "context_switch_cost", line_number)
    rescue_interval = _parse_int(values["rescue_interval"], "rescue_interval", line_number)
    turnaround = _parse_float(values["average_turnaround"], "average_turnaround", line_number)
    switches = _parse_int(values["context_switches"], "context_switches", line_number)
    jain = _parse_float(values["jain_slowdown"], "jain_slowdown", line_number)

    if total_processes <= 0:
        raise CampaignValidationError(
            f"linha {line_number}: total_processes deve ser positivo"
        )
    if min(total_time, quantum, switch_cost) < 0:
        raise CampaignValidationError(
            f"linha {line_number}: tempos e quantum devem ser não negativos"
        )
    if rescue_interval <= 0:
        raise CampaignValidationError(
            f"linha {line_number}: rescue_interval deve ser positivo"
        )
    if turnaround < 0:
        raise CampaignValidationError(
            f"linha {line_number}: average_turnaround deve ser não negativo"
        )
    if switches < 0:
        raise CampaignValidationError(
            f"linha {line_number}: context_switches deve ser não negativo"
        )
    if not 0.0 <= jain <= 1.0:
        raise CampaignValidationError(
            f"linha {line_number}: jain_slowdown deve estar entre 0 e 1"
        )
    return RunRecord(scenario, algorithm, seed, turnaround, switches, jain)
