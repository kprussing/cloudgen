add_executable(test-parser parser.c
    $<TARGET_OBJECTS:cloudgen-lib>
)
target_link_libraries(test-parser cloudgen-lib)
add_test(NAME test-parser
    COMMAND test-parser ${CMAKE_CURRENT_SOURCE_DIR}/../samples/cirrus.dat
)
