include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(cfg_parser_tests
    symbol_test.cpp
    prod_rule_test.cpp
    grammar_test.cpp
    grammar_traverser_test.cpp
    parser_test.cpp;
)

target_include_directories(cfg_parser_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(cfg_parser_tests
    PRIVATE cfg_parser
    GTest::gtest_main
)

Include(GoogleTest)
gtest_discover_tests(cfg_parser_tests)