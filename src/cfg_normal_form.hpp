#pragma once

#include "cfg_parser_impl.hpp"

namespace cfg_parser {

class CfgParser::Impl::Cnf {

    friend Normalizer;

public:
    std::vector<Cfg::Nonterminal> owned_nonterminals;

    Cnf(Impl* pimpl) : pimpl(pimpl) {}
    ~Cnf();

    void set(Cfg::Nonterminal nonterminal_to_normalize);
    const Cfg& get();

private:
    Cfg norm_form;
    Impl* pimpl;
    Cfg::Nonterminal nonterminal_to_normalize; 
    bool valid = false;

    void delete_owned_nonterminals();
};

}
