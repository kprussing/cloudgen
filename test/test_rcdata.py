import pathlib

from typing import (
    Dict,
)

from cloudgen import parse_input_file


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
