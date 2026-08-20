import csv
from contextlib import redirect_stderr
import io
import unittest

from scripts.consolidate_statistics import main
from tests.consolidation_test_support import CampaignFixture, campaign_rows

TABLES = (
    "average_turnaround.csv",
    "context_switches.csv",
    "jain_slowdown_percent.csv",
)


class CliTests(CampaignFixture, unittest.TestCase):
    def test_valid_campaign_writes_all_tables(self):
        self.write_campaign()
        output_dir = self.root / "processed"
        result = main([
            "--input", str(self.csv_path), "--seeds", str(self.seeds_path),
            "--output-dir", str(output_dir),
        ])
        self.assertEqual(result, 0)
        for name in TABLES:
            with (output_dir / name).open(newline="", encoding="utf-8") as table:
                self.assertEqual(sum(1 for _ in csv.reader(table)), 17)

    def test_invalid_campaign_preserves_existing_tables(self):
        self.write_campaign(campaign_rows()[:-1])
        output_dir = self.root / "processed"
        output_dir.mkdir()
        sentinels = {}
        for name in TABLES:
            path = output_dir / name
            path.write_text(f"sentinel-{name}", encoding="utf-8")
            sentinels[path] = path.read_text(encoding="utf-8")

        stderr = io.StringIO()
        with redirect_stderr(stderr):
            result = main([
                "--input", str(self.csv_path), "--seeds", str(self.seeds_path),
                "--output-dir", str(output_dir),
            ])
        self.assertNotEqual(result, 0)
        self.assertIn("Erro:", stderr.getvalue())
        self.assertIn("combinação ausente", stderr.getvalue())
        for path, expected in sentinels.items():
            self.assertEqual(path.read_text(encoding="utf-8"), expected)
