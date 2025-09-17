#pragma once

#include "parser_impl.hpp"

#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace cfg_parser {

class parser::impl::normalizer {

public:
    normalizer(gram_family& gram_fam);
    
    void normalize();

private:
    gram_family& gram_fam;
    // Mutable copies of all reachable nonts from gram_fam.gram
    std::unordered_map<nonterminal, std::unique_ptr<grammar>> mut_copies;

    using  nont_pair = std::pair<nonterminal, nonterminal>;
    struct nont_pair_hash { size_t operator()(const nont_pair&) const; };
    
    std::unordered_map<
        nont_pair,
        std::unique_ptr<grammar>,
        nont_pair_hash
    > nont_pair_map;

    void verify_no_reachable_empty_nonts();

    void create_deep_copy();

    std::unordered_set<grammar*> get_grams_w_empty_rule();
    void replace_reachable_empty_rules();

    void replace_unit_rules(nonterminal);
    void replace_reachable_unit_rules();

    nonterminal get_singleton_nont(terminal);
    nonterminal get_singleton_nont(const std::string& term_str);
    prod_rule nont_seq_eq_of(const prod_rule&);
    prod_rule nont_pair_eq_of(const prod_rule&);
    void convert_reachable_rules_into_pairs();

    void set_norm_form(); // populates gram_fam.norm_form's rule set
    void transfer_ownership_nonts(); // to gram_fam.owned_grams

    friend class parser_normalizer_test;
};

}