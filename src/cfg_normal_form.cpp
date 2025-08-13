#include "cfg_normalizer.hpp"

#include <stdexcept>

using namespace cfg_parser;

CfgParser::Impl::Cnf::
~Cnf() {
    delete_owned_nonterminals();
}

void CfgParser::Impl::Cnf::
delete_owned_nonterminals() {
    for (const auto nonterminal : owned_nonterminals) {
        delete nonterminal;
    }

    owned_nonterminals.clear();
}

void CfgParser::Impl::Cnf::
set(Cfg::Nonterminal nonterminal_to_normalize) {
    if (this->nonterminal_to_normalize != nullptr) {
        throw std::logic_error("Nonterminal to normalize is already initialized.");
    }

    if (nonterminal_to_normalize == nullptr) {
        throw std::invalid_argument("Nonterminal to normalize can't be null");
    }

    this->nonterminal_to_normalize = nonterminal_to_normalize;
}

const Cfg& CfgParser::Impl::Cnf::
get() {
    if (valid) return norm_form;

    delete_owned_nonterminals();
    norm_form.clear();
    valid = true;
    
    if (!nonterminal_to_normalize->is_empty()) {
        Normalizer normalizer(*this);
        normalizer.normalize();
    }

    return norm_form;
}