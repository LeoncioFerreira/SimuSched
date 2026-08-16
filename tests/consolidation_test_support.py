"""Shared fixtures for consolidation tests."""

import csv
from pathlib import Path
import tempfile

HEADER = [
    "algorithm", "seed", "scenario", "configuration", "total_processes",
    "total_simulated_time", "quantum", "context_switch_cost",
    "rescue_interval", "average_turnaround", "context_switches", "jain_slowdown",
]
SCENARIOS = ("balanced", "cpu_bound", "io_bound", "unbalanced_priorities")
ALGORITHMS = ("fcfs", "priority", "round-robin", "tej")


def campaign_rows():
    return [
        [
            algorithm, str(seed), scenario, f"configs/{scenario}.conf", "100",
            "5000", "4", "1", "20", f"{seed + 0.5:.6f}", str(seed % 17),
            "0.875000",
        ]
        for scenario in SCENARIOS
        for algorithm in ALGORITHMS
        for seed in range(1, 101)
    ]


class CampaignFixture:
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory(dir=Path(__file__).parent)
        self.root = Path(self.temp_dir.name)
        self.csv_path = self.root / "simulations.csv"
        self.seeds_path = self.root / "seeds.txt"
        self.seeds_path.write_text(
            "".join(f"{seed}\n" for seed in range(1, 101)), encoding="utf-8"
        )

    def tearDown(self):
        self.temp_dir.cleanup()

    def write_campaign(self, rows=None, header=None):
        with self.csv_path.open("w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file, lineterminator="\n")
            writer.writerow(HEADER if header is None else header)
            writer.writerows(campaign_rows() if rows is None else rows)
