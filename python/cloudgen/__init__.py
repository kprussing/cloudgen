"""
:program:`Cloudgen` Interface Bindings
======================================
"""

import os

from typing import (
    Optional,
    Sequence,
    Union,
)

from ._cloudgen import (
    parse_input_file,
)
from ._types import (
    RCData,
)


class Cloudgen:
    """Interface to the :program:`cloudgen` inputs and outputs.

    Attributes
    ----------

    rc_data: RCData
        The :program:`cloudgen` input parameters.

    """
    def __init__(self, path: Union[os.PathLike, str] = ""):
        self.rc_data: RCData = {}
        if path:
            self.rc_data = parse_input_file(path)

    @property
    def verbose(self) -> bool:
        """Enable verbose mode"""
        return self._bool_getter("verbose")

    @verbose.setter
    def verbose(self, value: Union[bool, str]) -> None:
        self._bool_setter("verbose", value)

    @property
    def x_domain_size(self) -> Optional[float]:
        """The X domain size (m)"""
        return self._real_getter("x_domain_size")

    @x_domain_size.setter
    def x_domain_size(self, value: Union[float, str]) -> None:
        self._real_setter("x_domain_size", value)

    @property
    def z_domain_size(self) -> Optional[float]:
        """The Z domain size (m)"""
        return self._real_getter("z_domain_size")

    @z_domain_size.setter
    def z_domain_size(self, value: Union[float, str]) -> None:
        self._real_setter("z_domain_size", value)

    @property
    def x_pixels(self) -> Optional[int]:
        """Number of pixels in the horizontal directions"""
        return self._int_getter("x_pixels")

    @x_pixels.setter
    def x_pixels(self, value: Union[int, str]) -> None:
        self._int_setter("x_pixels", value)

    @property
    def y_pixels(self) -> Optional[int]:
        """Number of pixels in the horizontal directions"""
        return self.x_pixels

    @property
    def z_pixels(self) -> Optional[int]:
        """Number of pixels in the vertical direction"""
        return self._int_getter("z_pixels")

    @z_pixels.setter
    def z_pixels(self, value: Union[int, str]) -> None:
        self._int_setter("z_pixels", value)

    @property
    def x_offset(self) -> Optional[float]:
        """The X offset (m)"""
        return self._real_getter("x_offset")

    @x_offset.setter
    def x_offset(self, value: Union[float, str]) -> None:
        self._real_setter("x_offset", value)

    @property
    def y_offset(self) -> Optional[float]:
        """The Y offset (m)"""
        return self._real_getter("y_offset")

    @y_offset.setter
    def y_offset(self, value: Union[float, str]) -> None:
        self._real_setter("y_offset", value)

    @property
    def z_offset(self) -> Optional[float]:
        """The Z offset (m)"""
        return self._real_getter("z_offset")

    @z_offset.setter
    def z_offset(self, value: Union[float, str]) -> None:
        self._real_setter("z_offset", value)

    @property
    def seed(self) -> Optional[int]:
        """The random seed for the number generator"""
        return self._int_getter("seed")

    @seed.setter
    def seed(self, value: Union[int, str]) -> None:
        self._int_setter("seed", value)

    @property
    def system_random_file(self) -> str:
        """Path to the system random file"""
        return self._str_getter("system_random_file")

    @system_random_file.setter
    def system_random_file(self, value: str) -> None:
        self._str_setter("system_random_file", value)

    @property
    def vertical_exponent(self) -> Optional[float]:
        """The vertical direction power spectrum exponent"""
        return self._real_getter("vertical_exponent")

    @vertical_exponent.setter
    def vertical_exponent(self, value: Union[float, str]) -> None:
        self._real_setter("vertical_exponent", value)

    @property
    def outer_scale(self) -> Optional[float]:
        """The power spectrum flattening distance (m)"""
        return self._real_getter("outer_scale")

    @outer_scale.setter
    def outer_scale(self, value: Union[float, str]) -> None:
        self._real_setter("outer_scale", value)

    @property
    def interp_height(self) -> Sequence[float]:
        """The heights for interpolated values"""
        return self._real_array_getter("interp_height")

    @interp_height.setter
    def interp_height(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("interp_height", value)

    @property
    def x_displacement(self) -> Sequence[float]:
        """Explicit X displacement (m)"""
        return self._real_array_getter("x_displacement")

    @x_displacement.setter
    def x_displacement(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("x_displacement", value)

    @property
    def y_displacement(self) -> Sequence[float]:
        """Explicit Y displacement (m)"""
        return self._real_array_getter("y_displacement")

    @y_displacement.setter
    def y_displacement(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("y_displacement", value)

    @property
    def v_wind(self) -> Sequence[float]:
        """X direction wind profile (m s⁻¹)"""
        return self._real_array_getter("v_wind")

    @v_wind.setter
    def v_wind(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("v_wind", value)

    @property
    def u_wind(self) -> Sequence[float]:
        """Y direction wind profile (m s⁻¹)"""
        return self._real_array_getter("u_wind")

    @u_wind.setter
    def u_wind(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("u_wind", value)

    @property
    def fall_speed(self) -> Sequence[float]:
        """Cloud particle fall speed (m s⁻¹)"""
        return self._real_array_getter("fall_speed")

    @fall_speed.setter
    def fall_speed(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("fall_speed", value)

    @property
    def generating_level(self) -> Optional[float]:
        """The generating level of the cloud (m)"""
        return self._real_getter("generating_level")

    @generating_level.setter
    def generating_level(self, value: Union[float, str]) -> None:
        self._real_setter("generating_level", value)

    @property
    def wind_scale_factor(self) -> Optional[float]:
        """The wind speed scaling factor"""
        return self._real_getter("wind_scale_factor")

    @wind_scale_factor.setter
    def wind_scale_factor(self, value: Union[float, str]) -> None:
        self._real_setter("wind_scale_factor", value)

    @property
    def horizontal_exponent(self) -> Sequence[float]:
        """Height variation of the exponent of the power spectrum"""
        return self._real_array_getter("horizontal_exponent")

    @horizontal_exponent.setter
    def horizontal_exponent(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("horizontal_exponent", value)

    @property
    def anisotropic_mixing(self) -> bool:
        """Enable anisotropic mixing"""
        return self._bool_getter("anisotropic_mixing")

    @anisotropic_mixing.setter
    def anisotropic_mixing(self, value: Union[bool, str]) -> None:
        self._bool_setter("anisotropic_mixing", value)

    @property
    def lognormal_distribution(self) -> bool:
        """Enable log-normal distributeion"""
        return self._bool_getter("lognormal_distribution")

    @lognormal_distribution.setter
    def lognormal_distribution(self, value: Union[bool, str]) -> None:
        self._bool_setter("lognormal_distribution", value)

    @property
    def mean(self) -> Sequence[float]:
        """Mean for the distribution"""
        return self._real_array_getter("mean")

    @mean.setter
    def mean(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("mean", value)

    @property
    def standard_deviation(self) -> Sequence[float]:
        """Standard deviation for the distribution"""
        return self._real_array_getter("standard_deviation")

    @standard_deviation.setter
    def standard_deviation(self, value: Union[Sequence[float], str]) -> None:
        self._real_array_setter("standard_deviation", value)

    @property
    def threshold(self) -> Optional[float]:
        """The minimum value for 'valid" data"""
        return self._real_getter("threshold")

    @threshold.setter
    def threshold(self, value: Union[float, str]) -> None:
        self._real_setter("threshold", value)

    @property
    def missing_value(self) -> Optional[float]:
        """The fill value in the output file"""
        return self._real_getter("missing_value")

    @missing_value.setter
    def missing_value(self, value: Union[float, str]) -> None:
        self._real_setter("missing_value", value)

    @property
    def output_filename(self) -> str:
        """Path to the output file"""
        return self._str_getter("output_filename")

    @output_filename.setter
    def output_filename(self, value: str) -> None:
        self._str_setter("output_filename", value)

    @property
    def variable_name(self) -> str:
        """Primary output variable name in the output file"""
        return self._str_getter("variable_name")

    @variable_name.setter
    def variable_name(self, value: str) -> None:
        self._str_setter("variable_name", value)

    @property
    def long_name(self) -> str:
        """The long description of the main output"""
        return self._str_getter("long_name")

    @long_name.setter
    def long_name(self, value: str) -> None:
        self._str_setter("long_name", value)

    @property
    def units(self) -> str:
        """The units of the primary output variable"""
        return self._str_getter("units")

    @units.setter
    def units(self, value: str) -> None:
        self._str_setter("units", value)

    @property
    def comment(self) -> str:
        """Additional comments to write to the output file"""
        return self._str_getter("comment")

    @comment.setter
    def comment(self, value: str) -> None:
        self._str_setter("comment", value)

    @property
    def references(self) -> str:
        """Additional references for the data"""
        return self._str_getter("references")

    @references.setter
    def references(self, value: str) -> None:
        self._str_setter("references", value)

    @property
    def institution(self) -> str:
        """The author's institution"""
        return self._str_getter("institution")

    @institution.setter
    def institution(self, value: str) -> None:
        self._str_setter("institution", value)

    @property
    def title(self) -> str:
        """The title of the output file"""
        return self._str_getter("title")

    @title.setter
    def title(self, value: str) -> None:
        self._str_setter("title", value)

    @property
    def user(self) -> str:
        """The user generating the output"""
        return self._str_getter("user")

    @user.setter
    def user(self, value: str) -> None:
        self._str_setter("user", value)

    def _bool_getter(self, param: str) -> bool:
        """Extract a boolean parameter"""
        return self.rc_data.get(param, "false").lower() not in ("0", "false")

    def _bool_setter(self, param: str, value: Union[str, bool]) -> None:
        """Set a boolean parameter"""
        if isinstance(value, str):
            self.rc_data[param] = value
        elif isinstance(value, bool):
            self.rc_data[param] = "true" if value else "false"
        else:
            raise ValueError(
                f"Could not convert {type(value)} to rc_data bool"
            )

    def _int_getter(self, param: str) -> Optional[int]:
        """Extract an integer parameter"""
        if param not in self.rc_data:
            return None
        else:
            return int(self.rc_data[param])

    def _int_setter(self, param: str, value: Union[str, int]) -> None:
        """Set an integer parameter"""
        try:
            _ = int(value)
        except ValueError:
            raise ValueError(
                f"Could not convert {type(value)} to rc_data int"
            )

        self.rc_data[param] = str(value)

    def _real_getter(self, param: str) -> Optional[float]:
        """Extract a real parameter"""
        if param not in self.rc_data:
            return None
        else:
            return float(self.rc_data[param])

    def _real_setter(self, param: str, value: Union[str, float]) -> None:
        """Set a real parameter"""
        try:
            _ = float(value)
        except ValueError:
            raise ValueError(
                f"Could not convert {type(value)} to rc_data real"
            )

        self.rc_data[param] = str(value)

    def _str_getter(self, param: str) -> str:
        """Extract a string parameter"""
        return self.rc_data.get(param, "")

    def _str_setter(self, param: str, value: str) -> None:
        """Set a string parameter"""
        self.rc_data[param] = value

    def _real_array_getter(self, param: str) -> Sequence[float]:
        """Extract an array of reals"""
        return [float(_) for _ in self.rc_data.get(param, "").split()]

    def _real_array_setter(self,
                           param: str,
                           value: Union[str, Sequence[float]]
                           ) -> None:
        """Set an array of reals"""
        try:
            if isinstance(value, str):
                _ = [float(_) for _ in value.split()]
                _value = value
            else:
                _ = [float(_) for _ in value]
                _value = " ".join([str(_) for _ in value])

        except ValueError:
            raise ValueError(
                f"Could not convert {value} to rc_data real array"
            )
        else:
            self.rc_data[param] = _value

    def __str__(self) -> str:
        """The text of the input file"""
        return "\n".join([" ".join(_) for _ in self.rc_data.items()])
