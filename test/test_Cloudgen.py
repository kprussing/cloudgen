import pathlib

import netCDF4
import numpy
import pytest

from cloudgen import Cloudgen, RCData


def test_init(cirrus: pathlib.Path,
              cirrus_expected: RCData,
              stratocumulus: pathlib.Path) -> None:
    """Check initialization works as expected"""
    empty = Cloudgen()
    assert empty.rc_data == {}
    assert not empty.verbose

    cc = Cloudgen(cirrus)
    assert cc.rc_data
    for param, value in cirrus_expected.items():
        assert param in cc.rc_data
        assert value.strip() == cc.rc_data[param].strip()
        assert hasattr(cc, param)
        if isinstance(getattr(cc, param), str):
            assert getattr(cc, param).strip() == value.strip()
        elif isinstance(getattr(cc, param), bool):
            assert getattr(cc, param) == (
                value.lower() not in ("0", "false")
            )
        elif isinstance(getattr(cc, param), int):
            assert getattr(cc, param) == int(value)
        elif isinstance(getattr(cc, param), float):
            assert getattr(cc, param) == float(value)
        elif isinstance(getattr(cc, param), list):
            assert len(getattr(cc, param)) == len(value.split())
            assert all(left == right for left, right
                       in zip(getattr(cc, param),
                              [float(_) for _ in value.split()]))

    assert all(_ in cirrus_expected for _ in cc.rc_data)

    ss = Cloudgen(stratocumulus)
    assert ss.rc_data

    assert ss is not cc
    assert ss.rc_data is not cc.rc_data

    with pytest.raises(ValueError):
        # Ignore with mypy to make sure the runtime behavior is right.
        ss.x_domain_size = [1.0, 2.0]  # type: ignore


def test_updated(cirrus: pathlib.Path) -> None:
    """Check updating a parameter triggers the updated flag"""
    cloud = Cloudgen(cirrus)
    assert not cloud.updated
    cloud.verbose = not cloud.verbose
    assert cloud.updated
    cloud.verbose = not cloud.verbose
    cloud.v_wind = numpy.arange(5)
    assert cloud.updated

    # Check resetting puts everything back to normal
    cloud.input = cirrus
    assert not cloud.updated


def _test_run_helper(cloud_dat: pathlib.Path,
                     sample_dir: pathlib.Path,
                     tmp_path: pathlib.Path) -> None:
    """Check that running :program:`cloudgen` works"""
    cloud = Cloudgen(cloud_dat)
    try:
        cloud.run(cwd=tmp_path, check=True)
    except Exception:
        raise
    else:
        assert tmp_path.joinpath(cloud_dat).exists()
        new = Cloudgen(tmp_path.joinpath(cloud_dat))
        for param, value in cloud.rc_data.items():
            assert param in new.rc_data
            assert value.strip() == new.rc_data[param].strip()

        assert all(_ in cloud.rc_data for _ in new.rc_data)
        output = tmp_path.joinpath(cloud.output_filename)
        assert output.exists()
        with netCDF4.Dataset(output, "r") as out:
            assert "variable_name" in cloud.rc_data
            variable = cloud.rc_data["variable_name"].strip()
            assert variable in out.variables
            reference = sample_dir.joinpath(new.output_filename)
            assert reference.exists()
            with netCDF4.Dataset(reference, "r") as ref:
                assert variable in ref.variables
                assert numpy.isclose(out[variable],
                                     ref[variable]).all()

    finally:
        if tmp_path.joinpath(cloud.output_filename).exists():
            tmp_path.joinpath(cloud.output_filename).unlink()


def test_run_cirrus(cirrus: pathlib.Path,
                    sample_dir: pathlib.Path,
                    tmp_path: pathlib.Path) -> None:
    """Check the cirrus clouds yield the expected results"""
    _test_run_helper(cirrus, sample_dir, tmp_path)


def test_run_stratucumulus(stratocumulus: pathlib.Path,
                           sample_dir: pathlib.Path,
                           tmp_path: pathlib.Path) -> None:
    """Check the stratocumulus clouds yield the expected results"""
    _test_run_helper(stratocumulus, sample_dir, tmp_path)


def test_str(cirrus: pathlib.Path, cirrus_expected: RCData) -> None:
    """Check converting to string preserves structure"""
    cloud = Cloudgen(cirrus)
    for line in str(cloud).splitlines():
        param, value = line.split(" ", 1)
        assert param in cirrus_expected
        assert value.strip() == cirrus_expected[param].strip()
