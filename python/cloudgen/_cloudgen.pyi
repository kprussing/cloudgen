from os import PathLike
from typing import (
    Union,
)

from ._types import RCData


def parse_input_file(file: Union[PathLike, str]) -> RCData: ...
