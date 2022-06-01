find_program(nccmp nccmp)
if (NOT nccmp)
    get_target_property(NETCDF_INC_DIR netCDF::netcdf INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(NETCDF_LIB_PATH netCDF::netcdf LOCATION)
    FetchContent_Declare(nccmp_cmake
        GIT_REPOSITORY https://gitlab.com/remikz/nccmp.git
        INSTALL_COMMAND ""
    )
    FetchContent_GetProperties(nccmp_cmake)
    if (NOT nccmp_cmake_POPULATED)
        FetchContent_Populate(nccmp_cmake)
        list(APPEND CMAKE_MODULE_PATH ${nccmp_cmake_SOURCE_DIR}/cmake/modules)
        add_subdirectory(${nccmp_cmake_SOURCE_DIR}
                         ${nccmp_cmake_BINARY_DIR})
    endif()
endif()

add_test(NAME cirrus
         COMMAND cloudgen::executable
                 ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
         )
add_test(NAME cirrus_with_effective_radius
         COMMAND cloudgen::executable output_filename=iwc_with_effective_radius.nc
                 ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus_with_effective_radius.dat
         )
add_test(NAME stratocumulus
         COMMAND cloudgen::executable
                 ${CMAKE_CURRENT_SOURCE_DIR}/../samples/stratocumulus.dat
         )

add_test(NAME cirrus_with_effective_radius-regression
         COMMAND nccmp -mdf
                ${CMAKE_CURRENT_SOURCE_DIR}/../samples/iwc_with_effective_radius.nc
                ${CMAKE_CURRENT_BINARY_DIR}/iwc_with_effective_radius.nc
         )
set_tests_properties(cirrus_with_effective_radius-regression PROPERTIES
                     DEPENDS "cirrus_with_effective_radius;nccmp-build")
add_test(NAME cirrus-regression
         COMMAND nccmp -mdf
                ${CMAKE_CURRENT_SOURCE_DIR}/../samples/iwc.nc
                ${CMAKE_CURRENT_BINARY_DIR}/iwc.nc
         )
set_tests_properties(cirrus-regression PROPERTIES DEPENDS "cirrus;nccmp-build")
add_test(NAME stratocumulus-regression
         COMMAND nccmp -mdf
                ${CMAKE_CURRENT_SOURCE_DIR}/../samples/lwc.nc
                ${CMAKE_CURRENT_BINARY_DIR}/lwc.nc
         )
set_tests_properties(stratocumulus-regression PROPERTIES DEPENDS "stratocumulus;nccmp-build")

add_executable(test-parser parser.c)
target_link_libraries(test-parser cloudgen::cloudgen)
add_test(NAME test-parser
    COMMAND test-parser ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
)

add_executable(parameter-overwrite parameter-overwrite.c)
target_link_libraries(parameter-overwrite cloudgen::cloudgen)
add_test(NAME parameter-overwrite-command-line
    COMMAND parameter-overwrite ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat verbose=overwrite
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
               ${CMAKE_CURRENT_BINARY_DIR}/cirrus-overwrite.dat)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/cirrus-overwrite.dat
    "verbose overwrite"
)
add_test(NAME parameter-overwrite-file
    COMMAND parameter-overwrite ${CMAKE_CURRENT_BINARY_DIR}/cirrus-overwrite.dat
)

add_executable(create-field create-field.c check-field.c)
target_link_libraries(create-field cloudgen::cloudgen)
add_test(NAME create-field-cirrus
         COMMAND create-field
         ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
         ${CMAKE_CURRENT_SOURCE_DIR}/cirrus-field.txt
         )

add_test(NAME create_with_effective_radius-field-cirrus
         COMMAND create-field
         ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus_with_effective_radius.dat
         ${CMAKE_CURRENT_SOURCE_DIR}/cirrus_with_effective_radius-field.txt
         )

add_executable(initial-phase initial-phase.c check-field.c)
target_link_libraries(initial-phase cloudgen::cloudgen)
add_test(NAME cirrus-initial-phase
    COMMAND initial-phase
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus-initial-phase.txt)

add_test(NAME stratocumulus-initial-phase
    COMMAND initial-phase
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/stratocumulus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/stratocumulus-initial-phase.txt)

add_test(NAME cirrus_with_effective_radius-initial-phase
    COMMAND initial-phase
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus_with_effective_radius.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus_with_effective_radius-initial-phase.txt)

add_executable(power-law power-law.c check-field.c)
target_link_libraries(power-law cloudgen::cloudgen)
add_test(NAME cirrus-power-law
    COMMAND power-law
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus-power-law.txt)

add_test(NAME stratocumulus-power-law
    COMMAND power-law
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/stratocumulus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/stratocumulus-power-law.txt)

add_test(NAME cirrus_with_effective_radius-power-law
    COMMAND power-law
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus_with_effective_radius.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus_with_effective_radius-power-law.txt)

add_executable(generate-fractal generate-fractal.c check-field.c)
target_link_libraries(generate-fractal cloudgen::cloudgen)
add_test(NAME cirrus-generate-fractal
    COMMAND generate-fractal
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus-generate-fractal.txt)

add_test(NAME stratocumulus-generate-fractal
    COMMAND generate-fractal
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/stratocumulus.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/stratocumulus-generate-fractal.txt)

add_test(NAME cirrus_with_effective_radius-generate-fractal
    COMMAND generate-fractal
            ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus_with_effective_radius.dat
            ${CMAKE_CURRENT_SOURCE_DIR}/cirrus_with_effective_radius-generate-fractal.txt)
