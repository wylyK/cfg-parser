add_library(cfgparser
    cfg_normal_form.cpp
    cfg_normalizer.cpp
    cfg_parser.cpp
    cfg_printer.cpp
    cfg_rule.cpp
    cfg.cpp
)

target_include_directories(cfgparser PUBLIC ${PROJECT_SOURCE_DIR}/include)
