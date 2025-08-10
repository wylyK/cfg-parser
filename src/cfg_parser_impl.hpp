#pragma once

#include "cfg_parser.hpp"

#include <utility>
#include <unordered_map>

namespace cfg_parser {

class CfgParser::Impl {

    friend CfgParser;
    class Cnf;
    class Normalizer;

    std::unordered_map<std::string, std::pair<Cfg, Cnf>> grammar_map;
    std::unordered_map<Cfg::Nonterminal, std::string> name_map;
    std::unordered_map<std::string, const Cfg> singleton_map;
    std::unordered_map<Cfg::Nonterminal, std::string> name_map_for_norm;

    bool  is_foreign(const Cfg::Nonterminal nonterminal) const;
    bool has_foreign_nonterminal(const Cfg::Rule& rule)  const;
};

}