enable_language(CXX)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c
        "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 REQUIRED)

add_library(_cloudgen MODULE
    cloudgen/_cloudgen.cpp
)
python_extension_module(_cloudgen)
target_link_libraries(_cloudgen pybind11::module cloudgen::cloudgen)
target_compile_options(_cloudgen
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -fvisibility=hidden
            -Wall
            -Wextra
            -pedantic
            -Werror
        >
)
install(TARGETS _cloudgen DESTINATION
        ${PYTHON_RELATIVE_SITE_PACKAGES_DIR}/cloudgen)

# Force the RPATH installation so the user does not have to fiddle
# with the library path `(DY)?LD_LIBRARY_PATH` 1_.  Setting it for the
# module is relevant primarily for the installation in a virtual
# environment.
#
# .. _1: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling#always-full-rpath
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${lib_path}" is_system)
if ("${is_system}" STREQUAL "-1")
    set_target_properties(_cloudgen PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${lib_path}")
endif()
