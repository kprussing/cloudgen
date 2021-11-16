import cloudgen


def test_add() -> None:
    """Initial test of setup"""
    assert cloudgen.add(1, 2) == 3


def test_hello() -> None:
    assert cloudgen.hello() == "Hello world"
