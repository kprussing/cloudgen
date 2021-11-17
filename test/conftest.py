import pathlib

from typing import (
    Dict,
)

import pytest


@pytest.fixture
def sample_dir() -> pathlib.Path:
    """The path to the samples directory"""
    return pathlib.Path(__file__).parent.parent / "samples"


@pytest.fixture
def cirrus(sample_dir) -> pathlib.Path:
    """The cirrus cloud input file"""
    return sample_dir / "cirrus.dat"


@pytest.fixture
def cirrus_expected(cirrus) -> Dict[str, str]:
    """Parse the expected cirrus cloud values"""
    return parse_input(cirrus)


@pytest.fixture
def stratocumulus(sample_dir) -> pathlib.Path:
    """The stratocumulus input file"""
    return sample_dir / "stratocumulus.dat"


@pytest.fixture
def stratocumulus_expected(stratocumulus) -> Dict[str, str]:
    """Parse the expected stratocumulus cloud values"""
    return parse_input(stratocumulus)


def parse_input(path: pathlib.Path) -> Dict[str, str]:
    """Parse the expected values from the file"""
    expected = {}
    with open(path, "r") as fid:
        for line in fid:
            if line.strip() == "" or line.lstrip().startswith("#"):
                continue

            try:
                param, value = line.strip().split(" ", 1)
            except ValueError:
                expected[line.strip()] = "true"
            else:
                expected[param] = value.strip()

    return expected
