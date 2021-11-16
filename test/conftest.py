import pathlib

import pytest


@pytest.fixture
def cirrus() -> pathlib.Path:
    """The cirrus cloud input file"""
    return pathlib.Path(__file__).parent.parent / "samples" / "cirrus.dat"
