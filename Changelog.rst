Changelog
=========

All notable changes to this project will be documented in this file.
The format is based on `Keep a Changelog`_.

Unreleased_
-----------

Added
^^^^^

Changed
^^^^^^^

Fixed
^^^^^

-   Inclusion of ncmpp for testing purposes

Removed
^^^^^^^

-   Conditional build of library and executable with Python bindings

1.7_ 2022-06-03
---------------

Added
^^^^^

-   Regression tests

Changed
^^^^^^^

-   Refactored the base field generation into a subprogram
-   Forced the order of random number selection

Fixed
^^^^^

-   Correctly set RPATH for Python installation
-   Set the minimum version of netCDF
-   Use the correct name to install FindFFTW.cmake

Removed
^^^^^^^

1.6_ 2022-05-16
---------------

Added
^^^^^

-   Install the samples, scripts, and example outputs
-   Python bindings to input and output files
-   CPack packaging

Changed
^^^^^^^

Fixed
^^^^^

-   Installed with ``RPATH`` set properly

Removed
-------

-   Unused internal functions

1.5_ 2021-11-18
---------------

Added
^^^^^

-   Tests for parsing
-   A change log

Changed
^^^^^^^

-   Parse input file using Bison and Flex
-   Expose ``rc_find`` as a public method
-   Build as a library with an executable
-   Install versioned library

Fixed
^^^^^

-   Add necessary headers to the sources
-   Use explicit print format for error reporting
-   Install CMake_ files to correct locations

Removed
^^^^^^^

-   Custom netCDF_ patch


1.4_ 2020-09-01
---------------

Added
^^^^^

-   CMake_ build system
-   Find FFTW_ cmake script

Changed
^^^^^^^

-   Explicitly set the C standard to C11

Fixed
^^^^^

-   Use the explicit C complex type

1.3_ 2019-10-24
---------------

Changed
^^^^^^^

-   Create a github repository for the source

.. _Unreleased: https://github.com/kprussing/cloudgen/compare/1.7...HEAD
.. _1.6: https://github.com/kprussing/cloudgen/compare/1.6...1.7
.. _1.5: https://github.com/kprussing/cloudgen/compare/1.5...1.6
.. _1.5: https://github.com/kprussing/cloudgen/compare/1.4...1.5
.. _1.4: https://github.com/kprussing/cloudgen/compare/1.3...1.4
.. _1.3: https://github.com/kprussing/cloudgen/releases/tag/1.3
.. _Keep a Changelog: https://keepachangelog.com/en/1.0.0/
.. _CMake: https://cmake.org
.. _FFTW: https://www.fftw.org
.. _netCDF: https://www.unidata.ucar.edu/software/netcdf/
