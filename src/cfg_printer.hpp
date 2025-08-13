#pragma once

#include "cfg_parser_impl.hpp"

#include <utility>

namespace cfg_parser {

class CfgPrinter {

protected:
    std::unordered_map<Cfg::Nonterminal, std::string>& name_map;
    
    virtual void print_rule(const Cfg::Rule& rule, int padding);
    virtual void print_shallow(Cfg::Nonterminal nonterminal);

public:
    std::pair<char, char> nonterminal_braces;    

    CfgPrinter(
        std::unordered_map<Cfg::Nonterminal, std::string>& name_map,
        std::pair<char, char> nonterminal_braces) :
            name_map(name_map), nonterminal_braces(nonterminal_braces) {}

    void operator()(Cfg::Nonterminal nonterminal);
};

}