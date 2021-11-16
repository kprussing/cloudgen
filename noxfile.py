import os
import pathlib
import re
import subprocess
import sys

import nox
import toml
from setuptools.config import read_configuration

if nox.options.default_venv_backend is None:
    try:
        subprocess.run(["conda"], check=True, capture_output=True)
    except subprocess.CalledProcessError:
        pass
    except FileNotFoundError:
        pass
    else:
        nox.options.default_venv_backend = "conda"

config = read_configuration(pathlib.Path(__file__).parent / "setup.cfg")

pythons = [v.split(":")[-1].strip()
           for v in config["metadata"]["classifiers"]
           if re.search(r"Python\s*::\s*\d+[.]\d+\s*$", v)
           ]
nox.options.sessions = [
    "lint",
    *["tests-" + x for x in pythons],
]
nox.options.error_on_external_run = True
nox.options.reuse_existing_virtualenvs = True


@nox.session
def lint(session):
    """Run the linters"""
    session.install("flake8", "mypy", "cpplint", "nox>=2020.1.8")
    session.run("flake8", "python", "test", "noxfile.py")
    session.run("mypy", "python", "test", "noxfile.py")
    sources = pathlib.Path(__file__).parent.glob("*.[hc]")
    session.run("cpplint", "--recursive", "src", *[str(_) for _ in sources],
                "test")


@nox.session(python=pythons)
def tests(session):
    """Run the tests"""
    pyproj = toml.load(pathlib.Path(__file__).parent / "pyproject.toml")
    deps = [_ for _ in pyproj["build-system"]["requires"]]

    deps.extend(config["options"].get("install_requires", []))
    deps.extend(
        config["options"].get("extras_require", {}).get("test", [])
    )
    if deps != []:
        session.install(*deps)

    # Remove any previous builds to make scikit-build happy.
    try:
        proc = subprocess.run([os.path.join(session.bin, "python"),
                               "-c",
                               "from skbuild import constants as c;" +
                               "print(c.CMAKE_BUILD_DIR())"
                               ],
                              check=True,
                              capture_output=True,
                              text=True,
                              env=session.env,
                              )
    except subprocess.CalledProcessError as err:
        session.error(err.stderr)
        raise

    builddir = proc.stdout.splitlines()[0]
    if proc.stderr != "":
        session.error(proc.stderr)

    env = {"Python_VER": session.python,
           "Python3_ROOT_DIR": os.path.dirname(session.bin)}
    session.install(".", "--use-feature=in-tree-build", env=env)
    if session.posargs:
        tests = session.posargs
    else:
        tests = ["-v", "test"]

    if "--ipdb" in tests:
        tests.remove("--ipdb")
        ipdb = ["--pdb", "--pdbcls=IPython.terminal.debugger:TerminalPdb"]
        tests.extend(x for x in ipdb if x not in tests)
        session.install("ipython")

    session.run("pytest", *tests)

    # Run the CMake tests
    session.chdir(builddir)
    session.run("ctest", "--output-on-failure")


@nox.session(python=pythons)
def dist(session):
    """Build the distributions"""
    pyproj = toml.load(pathlib.Path(__file__).parent / "pyproject.toml")
    deps = [_ for _ in pyproj["build-system"]["requires"]]
    session.install(*deps)

    env = {"Python_VER": session.python,
           "Python3_ROOT_DIR": os.path.dirname(session.bin)}
    session.run("python", "setup.py", "bdist_wheel", env=env)
    if sys.version.startswith(session.python):
        session.run("python", "setup.py", "sdist")
