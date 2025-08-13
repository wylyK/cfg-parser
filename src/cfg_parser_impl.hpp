#pragma once

#include "cfg_parser.hpp"

#include <utility>
#include <unordered_map>

namespace cfg_parser {

class CfgParser::Impl {

    friend CfgParser;
    
    class Cnf {

    public:
        std::vector<Cfg::Nonterminal> owned_nonterminals;

        Cnf(Impl* pimpl) : pimpl(pimpl) {}
        ~Cnf();

        void set(Cfg::Nonterminal nonterminal_to_normalize);
        const Cfg& get();

    private:

        class Normalizer;

        Cfg norm_form;
        Impl* pimpl;
        Cfg::Nonterminal nonterminal_to_normalize; 
        bool valid = false;

        void delete_owned_nonterminals();
    };

    std::unordered_map<std::string, std::pair<Cfg, Cnf>> grammar_map;
    std::unordered_map<Cfg::Nonterminal, std::string> name_map;
    std::unordered_map<std::string, const Cfg> singleton_map;
    std::unordered_map<Cfg::Nonterminal, std::string> name_map_for_norm;

    bool  is_foreign(const Cfg::Nonterminal nonterminal) const;
    bool has_foreign_nonterminal(const Cfg::Rule& rule)  const;
};

}