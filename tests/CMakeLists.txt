include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(cfgparser_tests
    cfg_helpers_test.cpp
)

target_link_libraries(cfgparser_tests
    PRIVATE cfgparser
)

include(GoogleTest)
gtest_discover_tests(cfgparser_tests)