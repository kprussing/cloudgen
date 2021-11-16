enable_language(CXX)
# Force the Python to search the virtual environment first
set(Python3_FIND_VIRTUALENV FIRST)
set(Python3_FIND_FRAMEWORK LAST)
set(Python3_FIND_REGISTRY LAST)

if (DEFINED ENV{Python_VER})
    find_package(Python3 $ENV{Python_VER} EXACT
                 REQUIRED Interpreter Development)
else()
    find_package(Python3 REQUIRED Interpreter Development)
endif()
find_package(PythonExtensions REQUIRED)

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
