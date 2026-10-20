"""Regression checks: optimized Python must reject damaged research evidence.

Run separately from simulator tests:
    python3 -m unittest discover -s analysis -p 'test_plot_validation.py' -v
All corruption happens in temporary fixture copies, never in the research inputs.
"""
from __future__ import annotations

import ast
import csv
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "analysis/plot_results.py"


class OptimizedEvidenceValidationTests(unittest.TestCase):
    def fixture(self, path):
        (path / "analysis").mkdir()
        shutil.copy2(SCRIPT, path / "analysis/plot_results.py")
        shutil.copy2(ROOT / "results_archive.zip", path / "results_archive.zip")
        shutil.copytree(ROOT / "data", path / "data")
        return path / "analysis/plot_results.py"

    def execute_modes(self, script, expected_error=None):
        for flag, optimization in (("-O", None), (None, "1")):
            with self.subTest(flag=flag, PYTHONOPTIMIZE=optimization):
                environment = os.environ.copy()
                environment.pop("PYTHONOPTIMIZE", None)
                if optimization:
                    environment["PYTHONOPTIMIZE"] = optimization
                command = [sys.executable] + ([flag] if flag else []) + [str(script), "--verify-only"]
                result = subprocess.run(command, capture_output=True, text=True, env=environment, check=False)
                if expected_error is None:
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertIn("PASS: 8 archived policy runs", result.stdout)
                else:
                    self.assertNotEqual(result.returncode, 0, result.stdout)
                    self.assertIn("ValueError: " + expected_error, result.stderr)
                    self.assertNotIn("PASS:", result.stdout)

    def rewrite_member(self, path, name, transform):
        archive = path / "results_archive.zip"
        with zipfile.ZipFile(archive) as original:
            entries = [(info, transform(original.read(info.filename)) if info.filename == name else original.read(info.filename)) for info in original.infolist()]
        with zipfile.ZipFile(archive, "w") as changed:
            for info, data in entries:
                changed.writestr(info, data)

    def test_clean_sources_pass_with_both_optimization_modes(self):
        self.execute_modes(SCRIPT)

    def test_numeric_summary_corruption_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="hotcold-figure-fixture-") as directory:
            root = Path(directory)
            script = self.fixture(root)
            summary = root / "data/policy_summary_wide.csv"
            with summary.open(newline="") as stream:
                reader = csv.DictReader(stream)
                fields, rows = reader.fieldnames, list(reader)
            rows[0]["write_amplification_factor_baseline"] = str(float(rows[0]["write_amplification_factor_baseline"]) + .25)
            with summary.open("w", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                writer.writerows(rows)
            self.execute_modes(script, "Numeric mismatch:")

    def test_historical_phase_plan_corruption_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="hotcold-figure-fixture-") as directory:
            root = Path(directory)
            script = self.fixture(root)

            def corrupt(data):
                manifest = json.loads(data)
                manifest["phase_plan"]["warmup_rounds"] = 0
                return json.dumps(manifest).encode()

            self.rewrite_member(root, "results/prxy_1_hot_cold_threshold_8/baseline/experiment_manifest.json", corrupt)
            self.execute_modes(script, "Validation failed: manifest[\"phase_plan\"][\"warmup_rounds\"] == 1")

    def test_trace_checksum_corruption_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="hotcold-figure-fixture-") as directory:
            root = Path(directory)
            script = self.fixture(root)
            self.rewrite_member(root, "results/alibaba_candidate_100k/device_4.standard.csv", lambda data: data + b"\n")
            self.execute_modes(script, "Validation failed: hashlib.sha256(raw).hexdigest() == trace[\"sha256\"]")

    def test_validation_has_no_optimization_removable_asserts(self):
        checks = [node for node in ast.walk(ast.parse(SCRIPT.read_text())) if isinstance(node, ast.Assert)]
        self.assertEqual(checks, [], "Evidence validation must use explicit exceptions, never assert.")


if __name__ == "__main__":
    unittest.main()
