import math
from pathlib import Path
import shutil
import subprocess
import unittest

from scripts.consolidate_statistics import student_t_critical_95, summarize


class ArchitectureTests(unittest.TestCase):
    def test_task_python_files_have_at_most_180_lines(self):
        root = Path(__file__).parents[1]
        files = [root / "scripts" / "consolidate_statistics.py"]
        files.extend((root / "scripts").glob("consolidation_*.py"))
        files.append(root / "tests" / "consolidation_test_support.py")
        files.extend((root / "tests").glob("test_consolidate_*.py"))

        for path in files:
            with self.subTest(path=path.relative_to(root)):
                line_count = len(path.read_text(encoding="utf-8").splitlines())
                self.assertLessEqual(line_count, 180)


class StatisticsTests(unittest.TestCase):
    def test_summarizes_hand_calculated_sample(self):
        summary = summarize([1.0, 2.0, 3.0, 4.0])
        deviation = math.sqrt(5.0 / 3.0)
        margin = 3.182446305 * deviation / 2.0
        self.assertAlmostEqual(summary.mean, 2.5, places=12)
        self.assertAlmostEqual(summary.sample_standard_deviation, deviation, places=12)
        self.assertEqual(summary.n, 4)
        self.assertAlmostEqual(summary.ci95_lower, 2.5 - margin, places=9)
        self.assertAlmostEqual(summary.ci95_upper, 2.5 + margin, places=9)

    def test_uses_student_t_critical_value_for_campaign_groups(self):
        self.assertAlmostEqual(student_t_critical_95(99), 1.984216952, places=9)

    def test_rejects_less_than_two_observations(self):
        with self.assertRaisesRegex(ValueError, "at least two"):
            summarize([1.0])

    def test_rejects_non_finite_observations(self):
        with self.assertRaisesRegex(ValueError, "finite"):
            summarize([1.0, math.inf])


class MakefileIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.make = shutil.which("make") or shutil.which("mingw32-make")
        if cls.make is None:
            raise unittest.SkipTest("make não está disponível")

    def test_stats_target_runs_consolidator_with_default_paths(self):
        result = subprocess.run(
            [self.make, "-n", "stats"], cwd=Path(__file__).parents[1],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("scripts/consolidate_statistics.py", result.stdout)
        self.assertIn("data/raw/simulations.csv", result.stdout)
        self.assertIn("configs/seeds.txt", result.stdout)
        self.assertIn("data/processed", result.stdout)

    def test_test_stats_target_runs_python_suite(self):
        result = subprocess.run(
            [self.make, "-n", "test-stats"], cwd=Path(__file__).parents[1],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("unittest", result.stdout)
        self.assertIn("test_consolidate", result.stdout)


if __name__ == "__main__":
    unittest.main()
