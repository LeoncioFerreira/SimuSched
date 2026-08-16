import unittest

from scripts.consolidate_statistics import CampaignValidationError, load_campaign
from tests.consolidation_test_support import CampaignFixture, HEADER, campaign_rows


class CampaignValidationTests(CampaignFixture, unittest.TestCase):
    def test_accepts_complete_campaign(self):
        self.write_campaign()
        records = load_campaign(self.csv_path, self.seeds_path)
        self.assertEqual(len(records), 1600)
        self.assertEqual(
            (records[0].scenario, records[0].algorithm, records[0].seed),
            ("balanced", "fcfs", 1),
        )

    def test_rejects_invalid_header(self):
        self.write_campaign(header=HEADER[:-1])
        with self.assertRaisesRegex(CampaignValidationError, "cabeçalho"):
            load_campaign(self.csv_path, self.seeds_path)

    def test_rejects_duplicate_combination(self):
        rows = campaign_rows()
        rows[-1] = rows[0].copy()
        self.write_campaign(rows)
        with self.assertRaisesRegex(
            CampaignValidationError, "combinação duplicada.*balanced.*fcfs.*1"
        ):
            load_campaign(self.csv_path, self.seeds_path)

    def test_rejects_missing_combination(self):
        self.write_campaign(campaign_rows()[:-1])
        with self.assertRaisesRegex(CampaignValidationError, "1 combinação ausente"):
            load_campaign(self.csv_path, self.seeds_path)

    def test_rejects_unexpected_scenario(self):
        rows = campaign_rows()
        rows[0][2] = "unknown"
        self.write_campaign(rows)
        with self.assertRaisesRegex(CampaignValidationError, "cenário inesperado"):
            load_campaign(self.csv_path, self.seeds_path)

    def test_rejects_invalid_metric_domains(self):
        for column, invalid, message in (
            (9, "nan", "average_turnaround.*finito"),
            (10, "-1", "context_switches.*não negativo"),
            (11, "1.1", "jain_slowdown.*entre 0 e 1"),
        ):
            with self.subTest(column=column):
                rows = campaign_rows()
                rows[0][column] = invalid
                self.write_campaign(rows)
                with self.assertRaisesRegex(CampaignValidationError, message):
                    load_campaign(self.csv_path, self.seeds_path)
