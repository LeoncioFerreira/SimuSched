"""Publish consolidated CSV tables atomically."""

import csv
import os
from pathlib import Path
import tempfile
from typing import Mapping, Sequence

if __package__:
    from .consolidation_models import OUTPUT_HEADER
else:
    from consolidation_models import OUTPUT_HEADER


def _format_output_value(field: str, value: object) -> object:
    if field in {"mean", "sample_standard_deviation", "ci95_lower", "ci95_upper"}:
        return f"{float(value):.9f}"
    return value


def write_tables_atomically(
    tables: Mapping[str, Sequence[Mapping[str, object]]], output_dir: Path
) -> None:
    """Write every table to temporary files before publishing any of them."""
    output_dir.mkdir(parents=True, exist_ok=True)
    temporary_paths = {}
    try:
        for filename, rows in tables.items():
            with tempfile.NamedTemporaryFile(
                mode="w", newline="", encoding="utf-8", dir=output_dir,
                prefix=f".{filename}.", suffix=".tmp", delete=False,
            ) as temporary_file:
                writer = csv.DictWriter(
                    temporary_file, fieldnames=OUTPUT_HEADER, lineterminator="\n"
                )
                writer.writeheader()
                for row in rows:
                    writer.writerow({
                        field: _format_output_value(field, row[field])
                        for field in OUTPUT_HEADER
                    })
                temporary_file.flush()
                os.fsync(temporary_file.fileno())
                temporary_paths[filename] = Path(temporary_file.name)
        for filename, temporary_path in temporary_paths.items():
            os.replace(temporary_path, output_dir / filename)
    finally:
        for temporary_path in temporary_paths.values():
            try:
                temporary_path.unlink()
            except FileNotFoundError:
                pass
