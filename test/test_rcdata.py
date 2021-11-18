import pathlib

import numpy

from typing import (
    Dict,
    List,
    Union,
)

from cloudgen import parse_input_file, rc_value


def test_parse_cirrus(cirrus: pathlib.Path,
                      cirrus_expected: Dict[str, str]) -> None:
    """Check parsing the cirrus input file works as expected"""
    rc_data = parse_input_file(cirrus)
    assert rc_data
    for param, value in cirrus_expected.items():
        assert param in rc_data
        assert value.strip() == rc_data[param].strip()

    assert all(_ in cirrus_expected for _ in rc_data)


def test_parse_stratocumulus(stratocumulus: pathlib.Path,
                             stratocumulus_expected: Dict[str, str]) -> None:
    """Check parsing the startocumulus input file works as expected"""
    rc_data = parse_input_file(stratocumulus)
    assert rc_data
    for param, value in stratocumulus_expected.items():
        assert param in rc_data
        assert value.strip() == rc_data[param].strip()

    assert all(_ in stratocumulus_expected for _ in rc_data)


def test_reparse(cirrus: pathlib.Path,
                 cirrus_expected: Dict[str, str],
                 stratocumulus: pathlib.Path,
                 stratocumulus_expected: Dict[str, str],
                 ) -> None:
    """Check that parsing a second file does not crash"""
    rc1 = parse_input_file(cirrus)
    rc2 = parse_input_file(stratocumulus)
    assert rc1 is not rc2
    for param, value in cirrus_expected.items():
        assert param in rc1
        assert value.strip() == rc1[param].strip()

    assert all(_ in cirrus_expected for _ in rc1)

    for param, value in stratocumulus_expected.items():
        assert param in rc2
        assert value.strip() == rc2[param].strip()

    assert all(_ in stratocumulus_expected for _ in rc2)


def test_conversions(cirrus: pathlib.Path) -> None:
    """Check the value conversions work as described"""
    rc_data = parse_input_file(cirrus)
    result: Union[bool, int, float, str, List[float]]

    param = "verbose"
    assert param in rc_data
    result = rc_value(rc_data[param])
    assert isinstance(result, bool)
    assert result

    result = rc_value("0")
    assert isinstance(result, bool)
    assert not result

    param = "x_pixels"
    assert param in rc_data
    result = rc_value(rc_data[param])
    assert isinstance(result, int)
    assert result == 256

    result = rc_value("  256 ")
    assert isinstance(result, int)
    assert result == 256

    param = "missing_value"
    assert param in rc_data
    result = rc_value(rc_data[param])
    assert isinstance(result, float)
    assert numpy.isclose(result, -999.0)

    expected: Union[str, List[float]] = "This is a test"
    assert isinstance(expected, str)
    result = rc_value(expected)
    assert isinstance(result, str)
    assert result == expected

    expected = [10, 15, 20, 25, 30, 35, 40, 40]
    param = "v_wind"
    assert param in rc_data
    result = rc_value(rc_data[param])
    assert isinstance(result, list)
    assert len(result) == len(expected)
    assert numpy.isclose(expected, result).all()
