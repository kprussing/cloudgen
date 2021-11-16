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
