add_library(cfg_parser
    grammar.cpp
    parser_impl_normalizer.cpp
    parser_impl.cpp
    parser.cpp
    prod_rule.cpp
)

target_include_directories(cfg_parser
    PUBLIC ${PROJECT_SOURCE_DIR}/include/cfg-parser
)
