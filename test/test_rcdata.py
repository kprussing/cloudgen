import pathlib

from cloudgen import parse_input_file


def test_parse_input_file(cirrus: pathlib.Path) -> None:
    """Check parsing the input file works as expected"""
    rc_data = parse_input_file(cirrus)
    assert rc_data
    expected = {
        "verbose": "true",
        "x_domain_size": "200000",
        "z_domain_size": "3500",
        "x_pixels": "256",
        "z_pixels": "32",
        "z_offset": "7000",
        "seed": "2",
        "vertical_exponent": "-2",
        "outer_scale": "50000",
        "interp_height": "7000 7500 8000 8500 9000 9500 10000 10500",
        "v_wind": "10 15 20 25 30 35 40 40",
        "u_wind": "5 2.5 0 -1 0 2.5 5 5",
        "fall_speed": "1 0.9 0.8 0.7 0.6 0.5 0.4 0.3",
        "generating_level": "9500",
        "horizontal_exponent": "-1.667 -3 -2.5 -2 -1.667 -1.667 -1.667 -1.667",
        "anisotropic_mixing": "true",
        "lognormal_distribution": "true",
        "mean": "0.0 0.06 0.05 0.04 0.02 0.01 0.0001 0",
        "standard_deviation": "2 1 1.25 1.5 1.75 2 2.1 2.2",
        "threshold": "0.001",
        "missing_value": "-999.0",
        "output_filename": "iwc.nc",
        "variable_name": "iwc",
        "long_name": "Ice water content",
        "units": "g m-3",
        "title": "3D stochastic field of ice water content",
    }
    for param, value in expected.items():
        assert param in rc_data
        assert value.strip() == rc_data[param].strip()
