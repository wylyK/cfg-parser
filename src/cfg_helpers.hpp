#pragma once

#include "cfg.hpp"

namespace cfg_parser {

inline bool is_terminal(Cfg::Symbol symbol) {
    return std::holds_alternative<Cfg::Terminal>(symbol);
}

inline bool is_nonterminal(Cfg::Symbol symbol) {
    return std::holds_alternative<Cfg::Nonterminal>(symbol);
}

inline Cfg::Terminal as_terminal(Cfg::Symbol symbol) {
    return std::get<Cfg::Terminal>(symbol);
}

inline Cfg::Nonterminal as_nonterminal(Cfg::Symbol symbol) {
    return std::get<Cfg::Nonterminal>(symbol);
}

}