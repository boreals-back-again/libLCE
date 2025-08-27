set(DOCTEST_FILES
    ../external/doctest/doctest/doctest.h
)

set(TEST_FILES
    #src/Archive.cpp
)

add_executable(tests ${TEST_FILES}).

target_include_directories(tests PRIVATE ${DOCTEST_FILES})
target_link_libraries(tests PRIVATE libLCE)

include(CTest)
enable_testing()