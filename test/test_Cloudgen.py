import pathlib

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


def test_str(cirrus: pathlib.Path, cirrus_expected: RCData) -> None:
    """Check converting to string preserves structure"""
    cloud = Cloudgen(cirrus)
    for line in str(cloud).splitlines():
        param, value = line.split(" ", 1)
        assert param in cirrus_expected
        assert value.strip() == cirrus_expected[param].strip()

