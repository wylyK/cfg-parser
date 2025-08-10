#pragma once

#include "cfg_parser_impl.hpp"

namespace cfg_parser {

class CfgParser::Impl::Normalizer {

public:
    Normalizer(Cnf& norm);
    
    void normalize();

private:
    Cnf& norm;
    std::unordered_map<Cfg::Nonterminal, Cfg*> mut_copy;

    using  NonterminalPair = std::pair<Cfg::Nonterminal, Cfg::Nonterminal>;
    struct NonterminalPairHash {
        size_t operator()(const NonterminalPair& pair) const;
    };
    std::unordered_map<
        NonterminalPair,
        Cfg::Nonterminal,
        NonterminalPairHash
    > nonterminal_pair_map;

    // Verify that no nonterminals reachable from nonterminal_to_normalize are empty
    void verify_no_reachable_empty_nonterminals();

    Cfg::Rule nonterminals_replaced_w_copy(const Cfg::Rule& rule);
    void create_copy(Cfg::Nonterminal curr);
    void create_deep_copy(); // of nonterminal_to_normalize and store in mut_copy

    // A rule R is reachable from a nonterminal A
    // if R belongs to a nonterminal reachable from A

    std::vector<Cfg*> get_nonterminals_w_empty_rule();
    void replace_reachable_empty_rules();

    void replace_unit_rules(Cfg::Nonterminal curr);
    void replace_reachable_unit_rules();

    Cfg::Nonterminal get_singleton_nonterminal(Cfg::Terminal terminal);
    Cfg::Nonterminal get_singleton_nonterminal(std::string terminal_str);
    Cfg::Rule nonterminal_pair_equiv_of(const Cfg::Rule& rule);
    void convert_reachable_rules_into_pairs();

    void transfer_ownership_of_nonterminals(); // to norm.owned_nonterminals

    void set_norm_form();
    // to populate norm.norm_form's rule set
};

}