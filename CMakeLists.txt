cmake_minimum_required(VERSION 3.6.1)

cmake_policy(SET CMP0048 NEW)
project(Cloudgen VERSION 1.3 LANGUAGES C)

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
        BINARY_DIR        ""
        INSTALL_DIR       ""
        PATCH_COMMAND     git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/findFFTW.patch
    )
    FetchContent_GetProperties(fftw_cmake)
    if (NOT fftw_cmake_POPULATED)
        FetchContent_Populate(fftw_cmake)
        list(APPEND CMAKE_MODULE_PATH ${fftw_cmake_SOURCE_DIR})
    endif()
    find_package(FFTW REQUIRED)
endif()

find_package(netCDF QUIET)
if (NOT netCDF_FOUND)
    FetchContent_Declare(netcdf4
        GIT_REPOSITORY    "http://github.com/Unidata/netcdf-c.git"
        SOURCE_DIR        "external/netcdf4-c"
        CMAKE_ARGS        "-DBUILD_SHARED_LIBS=On -DENABLE_DOXYGEN=Off"#-DENABLE_TESTS=Off"
        PATCH_COMMAND     git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/netcdf.patch
    )
    FetchContent_GetProperties(netcdf4)
    if (NOT netcdf4_POPULATED)
        FetchContent_Populate(netcdf4)
        add_subdirectory(${netcdf4_SOURCE_DIR} ${netcdf4_BINARY_DIR})
    endif()
endif()

add_executable(cloudgen
    cloudgen_core.c
    cloudgen_layers.c
    readconfig.c
    random.c
    nctools.c
    main.c
)

set(C_COMPILER_ID ${CMAKE_C_COMPILER_ID})
target_compile_options(cloudgen
    PUBLIC
    PRIVATE
    $<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:CLANG>>:
        -Wall
        -Wextra
        -pedantic
    >
)
target_link_libraries(cloudgen FFTW::Double netcdf)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/cloudgen)
install(TARGETS cloudgen
    EXPORT cloudgen-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT cloudgen-targets
    FILE
        CloudgenTargets.cmake
    NAMESPACE
        Cloudgen::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CloudgenConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/CloudgenConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CloudgenConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CloudgenConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CloudgenConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

