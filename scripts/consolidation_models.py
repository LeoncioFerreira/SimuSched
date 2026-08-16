"""Shared data models and constants for campaign consolidation."""

from dataclasses import dataclass

CSV_HEADER = (
    "algorithm",
    "seed",
    "scenario",
    "configuration",
    "total_processes",
    "total_simulated_time",
    "quantum",
    "context_switch_cost",
    "rescue_interval",
    "average_turnaround",
    "context_switches",
    "jain_slowdown",
)
SCENARIOS = ("balanced", "cpu_bound", "io_bound", "unbalanced_priorities")
ALGORITHMS = ("fcfs", "priority", "round-robin", "tej")
OUTPUT_HEADER = (
    "scenario",
    "algorithm",
    "mean",
    "sample_standard_deviation",
    "n",
    "ci95_lower",
    "ci95_upper",
)
METRICS = {
    "average_turnaround.csv": ("average_turnaround", 1.0),
    "context_switches.csv": ("context_switches", 1.0),
    "jain_slowdown_percent.csv": ("jain_slowdown", 100.0),
}


@dataclass(frozen=True)
class Summary:
    mean: float
    sample_standard_deviation: float
    n: int
    ci95_lower: float
    ci95_upper: float


@dataclass(frozen=True)
class RunRecord:
    scenario: str
    algorithm: str
    seed: int
    average_turnaround: float
    context_switches: int
    jain_slowdown: float


class CampaignValidationError(ValueError):
    """Raised when an experiment campaign cannot be safely consolidated."""
