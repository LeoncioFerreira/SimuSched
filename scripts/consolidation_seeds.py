"""Load and validate the expected campaign seeds."""

from pathlib import Path

if __package__:
    from .consolidation_models import CampaignValidationError
else:
    from consolidation_models import CampaignValidationError


def load_expected_seeds(path: Path) -> tuple[int, ...]:
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise CampaignValidationError(
            f"não foi possível ler o arquivo de seeds: {path}"
        ) from error

    seeds = []
    for line_number, raw_value in enumerate(lines, start=1):
        value = raw_value.strip()
        if not value:
            continue
        try:
            seed = int(value)
        except ValueError as error:
            raise CampaignValidationError(
                f"seed inválida na linha {line_number}: {value!r}"
            ) from error
        if seed < 0:
            raise CampaignValidationError(
                f"seed deve ser não negativa na linha {line_number}: {seed}"
            )
        seeds.append(seed)

    if len(seeds) != 100:
        raise CampaignValidationError(
            f"esperadas exatamente 100 seeds; encontradas {len(seeds)}"
        )
    if len(set(seeds)) != len(seeds):
        raise CampaignValidationError("as 100 seeds devem ser distintas")
    return tuple(seeds)
