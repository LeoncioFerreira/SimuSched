import math
import unittest

from scripts.consolidate_statistics import aggregate_campaign, load_campaign
from tests.consolidation_test_support import CampaignFixture


class AggregationTests(CampaignFixture, unittest.TestCase):
    def test_aggregates_three_metrics_in_deterministic_order(self):
        self.write_campaign()
        tables = aggregate_campaign(load_campaign(self.csv_path, self.seeds_path))
        self.assertEqual(
            set(tables),
            {"average_turnaround.csv", "context_switches.csv", "jain_slowdown_percent.csv"},
        )
        self.assertTrue(all(len(rows) == 16 for rows in tables.values()))
        first = tables["average_turnaround.csv"][0]
        self.assertEqual((first["scenario"], first["algorithm"]), ("balanced", "fcfs"))
        self.assertAlmostEqual(first["mean"], 51.0, places=12)
        self.assertAlmostEqual(
            first["sample_standard_deviation"], math.sqrt(841.6666666666666), places=12
        )
        self.assertEqual(first["n"], 100)

    def test_scales_jain_statistics_to_percentage_points(self):
        self.write_campaign()
        tables = aggregate_campaign(load_campaign(self.csv_path, self.seeds_path))
        first = tables["jain_slowdown_percent.csv"][0]
        self.assertEqual(first["mean"], 87.5)
        self.assertEqual(first["sample_standard_deviation"], 0.0)
        self.assertEqual(first["ci95_lower"], 87.5)
        self.assertEqual(first["ci95_upper"], 87.5)
