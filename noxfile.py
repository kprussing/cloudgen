import os
import pathlib
import re
import shutil
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
    deps = config["options"].get("install_requires", [])
    for _ in config["options"].get("extras_require", {}).values():
        deps.extend(_)

    session.install("flake8", "mypy", "cpplint", *deps)
    session.run("flake8", "python", "test", "noxfile.py")
    session.run("mypy", "python", "test", "noxfile.py")
    session.run("cpplint", "--recursive", "python", "test")


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

    # Check the version of FFTW if possible
    installed_fftw = False
    try:
        proc = subprocess.run(["fftw-wisdom-to-conf", "-V"],
                              check=True,
                              capture_output=True,
                              text=True,
                              env=session.env,
                              )
    except subprocess.CalledProcessError as err:
        if nox.options.default_venv_backend == "conda":
            session.conda_install("fftw>=3.3.4")
            installed_fftw = True
        else:
            session.error(err.stderr)

    else:
        match = re.search(r"FFTW\s*version\s*(\d+([.]\d+([.]\d+)?)?)",
                          proc.stdout)
        if not match:
            session.warn("Could not determine FFTW version")
        else:
            version = tuple(int(_) for _ in match.group(1).split("."))
            if version < (3, 3, 4):
                if nox.options.default_venv_backend == "conda":
                    session.conda_install("fftw>=3.3.4")
                    installed_fftw = True
                else:
                    session.error(f"Insufficient FFTW version {version}")

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

    if os.path.exists(builddir):
        shutil.rmtree(builddir)

    env = {"Python_VER": session.python,
           "Python3_ROOT_DIR": os.path.dirname(session.bin)}
    if installed_fftw:
        env["FFTWDIR"] = os.path.dirname(session.bin)

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
