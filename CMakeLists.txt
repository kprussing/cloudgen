cmake_minimum_required(VERSION 3.6.1)

cmake_policy(SET CMP0048 NEW)
project(cloudgen VERSION 1.5 LANGUAGES C)

include(GNUInstallDirs)
if (NOT DEFINED INSTALL_CONFIGDIR)
    if (CMAKE_SYSTEM_NAME MATCHES Windows)
        set(INSTALL_CONFIGDIR CMake)
    elseif (CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(INSTALL_CONFIGDIR ${PROJECT_NAME}.framework/Resources/CMake)
    else()
        set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
    endif()
endif()

# Preemptively load FetchContent for adding external dependencies
include(FetchContent OPTIONAL RESULT_VARIABLE fc_LOADED)
if (${fc_LOADED} MATCHES NOTFOUND)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchContent.cmake)
endif()

find_package(FFTW QUIET)
if (NOT FFTW_FOUND)
    FetchContent_Declare(fftw_cmake
        GIT_REPOSITORY    "https://github.com/egpbos/findfftw.git"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
        INSTALL_COMMAND   ""
        TEST_COMMAND      ""
        SOURCE_DIR        "external/findFFTW"
        INSTALL_DIR       ""
    )
    FetchContent_GetProperties(fftw_cmake)
    if (NOT fftw_cmake_POPULATED)
        FetchContent_Populate(fftw_cmake)
        list(APPEND CMAKE_MODULE_PATH ${fftw_cmake_SOURCE_DIR})
    endif()
    find_package(FFTW REQUIRED)
    install(FILES
        ${fftw_cmake_SOURCE_DIR}/findFFTW.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif()

find_package(netCDF REQUIRED)

option(USE_FLOAT "Use single precision values" Off)
if (USE_FLOAT)
    add_definitions(-DFFTW_ENABLE_FLOAT)
    set(fftw_lib FFTW::Float)
else()
    set(fftw_lib FFTW::Double)
endif()

find_package(BISON 3.0.1 REQUIRED)
find_package(FLEX REQUIRED)

flex_target(lexer parser.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
bison_target(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
add_flex_bison_dependency(lexer parser)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_library(cloudgen SHARED
    cloudgen_core.c
    cloudgen_layers.c
    readconfig.c
    random.c
    nctools.c
    ${FLEX_lexer_OUTPUTS}
    ${BISON_parser_OUTPUTS}
)
target_compile_options(cloudgen
    PRIVATE
        $<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>:
            -Wall
            -Wextra
            -pedantic
        >
)
target_compile_definitions(cloudgen
    PUBLIC
        $<$<BOOL:${USE_FLOAT}>:FFTW_ENABLE_FLOAT>
)
target_include_directories(cloudgen
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cloudgen PUBLIC ${fftw_lib} netCDF::netcdf)
set_target_properties(cloudgen PROPERTIES
    C_STANDARD 11
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
add_library(cloudgen::cloudgen ALIAS cloudgen)

add_executable(executable main.c)
target_link_libraries(executable cloudgen)
set_target_properties(executable PROPERTIES OUTPUT_NAME cloudgen)
add_executable(cloudgen::executable ALIAS executable)

install(TARGETS cloudgen
                executable
    EXPORT cloudgen-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES cloudgen.h
              readconfig.h
              random.h
              ${CMAKE_CURRENT_BINARY_DIR}/config.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cloudgen
)

install(EXPORT cloudgen-targets
    FILE
        cloudgenTargets.cmake
    NAMESPACE
        cloudgen::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cloudgenConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/cloudgenConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cloudgenConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cloudgenConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cloudgenConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

enable_testing()
add_subdirectory(test)

if (SKBUILD)
    add_subdirectory(python)
endif()
