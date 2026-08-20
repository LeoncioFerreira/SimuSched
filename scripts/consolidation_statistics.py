"""Statistical primitives and campaign aggregation."""

import math
from statistics import mean, stdev
from typing import Sequence

if __package__:
    from .consolidation_models import ALGORITHMS, METRICS, SCENARIOS, RunRecord, Summary
else:
    from consolidation_models import ALGORITHMS, METRICS, SCENARIOS, RunRecord, Summary

_T_CRITICAL_95 = {
    1: 12.706204736, 2: 4.302652730, 3: 3.182446305, 4: 2.776445105,
    5: 2.570581836, 6: 2.446911851, 7: 2.364624252, 8: 2.306004135,
    9: 2.262157163, 10: 2.228138852, 11: 2.200985160, 12: 2.178812830,
    13: 2.160368656, 14: 2.144786688, 15: 2.131449546, 16: 2.119905299,
    17: 2.109815578, 18: 2.100922040, 19: 2.093024054, 20: 2.085963447,
    21: 2.079613845, 22: 2.073873068, 23: 2.068657610, 24: 2.063898562,
    25: 2.059538553, 26: 2.055529439, 27: 2.051830516, 28: 2.048407142,
    29: 2.045229642, 30: 2.042272456, 40: 2.021075390, 60: 2.000297822,
    80: 1.990063421, 99: 1.984216952, 120: 1.979930406,
}


def student_t_critical_95(degrees_of_freedom: int) -> float:
    """Return a two-sided 95% Student t critical value."""
    if degrees_of_freedom < 1:
        raise ValueError("degrees of freedom must be positive")
    if degrees_of_freedom in _T_CRITICAL_95:
        return _T_CRITICAL_95[degrees_of_freedom]
    points = sorted(_T_CRITICAL_95)
    if degrees_of_freedom > points[-1]:
        raise ValueError("degrees of freedom exceed the supported range")
    lower = max(point for point in points if point < degrees_of_freedom)
    upper = min(point for point in points if point > degrees_of_freedom)
    fraction = (degrees_of_freedom - lower) / (upper - lower)
    return _T_CRITICAL_95[lower] + fraction * (
        _T_CRITICAL_95[upper] - _T_CRITICAL_95[lower]
    )


def summarize(values: Sequence[float]) -> Summary:
    """Calculate mean, sample deviation, sample size and two-sided 95% CI."""
    observations = [float(value) for value in values]
    if len(observations) < 2:
        raise ValueError("at least two observations are required")
    if not all(math.isfinite(value) for value in observations):
        raise ValueError("all observations must be finite")
    sample_mean = mean(observations)
    deviation = stdev(observations)
    critical = student_t_critical_95(len(observations) - 1)
    margin = critical * deviation / math.sqrt(len(observations))
    return Summary(
        sample_mean, deviation, len(observations),
        sample_mean - margin, sample_mean + margin,
    )


def aggregate_campaign(
    records: Sequence[RunRecord],
) -> dict[str, list[dict[str, object]]]:
    """Aggregate each metric by scenario and algorithm."""
    groups = {
        (scenario, algorithm): []
        for scenario in SCENARIOS
        for algorithm in ALGORITHMS
    }
    for record in records:
        groups[(record.scenario, record.algorithm)].append(record)
    tables = {}
    for filename, (attribute, scale) in METRICS.items():
        rows = []
        for scenario in SCENARIOS:
            for algorithm in ALGORITHMS:
                values = [
                    float(getattr(record, attribute)) * scale
                    for record in groups[(scenario, algorithm)]
                ]
                summary = summarize(values)
                rows.append({
                    "scenario": scenario, "algorithm": algorithm,
                    "mean": summary.mean,
                    "sample_standard_deviation": summary.sample_standard_deviation,
                    "n": summary.n, "ci95_lower": summary.ci95_lower,
                    "ci95_upper": summary.ci95_upper,
                })
        tables[filename] = rows
    return tables
