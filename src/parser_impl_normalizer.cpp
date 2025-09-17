#include "parser_impl_normalizer.hpp"

#include <unordered_set>
#include <algorithm>
#include <utility>
#include <algorithm>
#include <stdexcept>

using std::vector;
using std::string;
using std::unordered_set;
using std::pair;
using std::find;
using std::find_if;
using std::distance;

using namespace cfg_parser;

parser::impl::normalizer::
normalizer(gram_family& gram_fam) : gram_fam(gram_fam) {}

/* Verifies whether there aren't any nonts
reachable from gram_fam.gram are empty */
void parser::impl::normalizer::
verify_no_reachable_empty_nonts() {
    gram_fam.gram.dfs(
        [&](nonterminal nont) {
            if (nont->is_empty())
                throw std::logic_error(gram_fam.pimpl->name_map[nont] + " is empty.");
        }
    );
}

// Invokes gram_fam.gram.deep_copy to populate mut_copies
void parser::impl::normalizer::
create_deep_copy() {
    mut_copies = gram_fam.gram.deep_copy();
}

unordered_set<grammar*> parser::impl::normalizer::
get_grams_w_empty_rule() {
    unordered_set<grammar*> result;
    for (const auto& [_, ptr_copy_gram] : mut_copies) {
        if (!ptr_copy_gram->contains("")) continue;
        result.insert(ptr_copy_gram.get());
    }

    return result;
}

namespace cfg_parser::internal_parser_normalizer {
// __Helpers for replace_reachable_empty_rules__

void insert_pruned_rules(
    grammar& copy_gram,
    prod_rule::const_iterator beg,
    prod_rule::const_iterator end,
    nonterminal nont // to prune
) {
    const auto pos = find(beg, end, nont);
    if (pos == end) return;

    prod_rule pruned(beg, end);
    const auto prune_pos = pruned.begin() + distance(beg, pos);
    pruned.erase(prune_pos);

    if (pruned.size()  != 1 ||
        pruned.front() != nonterminal(copy_gram)
    ) copy_gram.insert(pruned); // if rule isn't redundant

    insert_pruned_rules(copy_gram, pos + 1, end, nont);
    insert_pruned_rules(copy_gram, prune_pos, pruned.end(), nont);
}

void insert_nempty_pruned_rules(
    grammar& copy_gram,
    prod_rule::const_iterator beg,
    prod_rule::const_iterator end,
    nonterminal nont // to prune
) {
    const auto pos = find(beg, end, nont);
    if (pos == end) return;

    prod_rule pruned(beg, end);
    const auto prune_pos = pruned.begin() + distance(beg, pos);
    pruned.erase(prune_pos);

    if (!pruned.is_empty() &&
        (pruned.size()  != 1 || pruned.front() != nonterminal(copy_gram))
    ) copy_gram.insert(pruned); // if rule isn't redundant

    insert_nempty_pruned_rules(copy_gram, pos + 1, end, nont);
    insert_nempty_pruned_rules(copy_gram, prune_pos, pruned.end(), nont);
}

void insert_all_pruned(
    grammar& copy_gram,
    nonterminal nont // to prune
) {
    for (const auto& rule : copy_gram) {
        insert_pruned_rules(
            copy_gram, rule.begin(), rule.end(), nont
        );
    }
}

void insert_all_nempty_pruned(
    grammar& copy_gram,
    nonterminal nont // to prune
) {
    for (const auto& rule : copy_gram) {
        insert_nempty_pruned_rules(
            copy_gram, rule.begin(), rule.end(), nont
        );
    }
}

} // End of namespace internal_parser_normalizer

void parser::impl::normalizer::
replace_reachable_empty_rules() {
    unordered_set<grammar*> grams_w_empty_rule = get_grams_w_empty_rule();
    unordered_set<const grammar*> grams_w_erased_empty_rule;
    while (!grams_w_empty_rule.empty()) {
        for (auto w_empty_rule : grams_w_empty_rule) {
            w_empty_rule->erase("");
            grams_w_erased_empty_rule.insert(w_empty_rule);
            for (auto& [_, ptr_copy_gram] : mut_copies) {
                auto& copy_gram = *ptr_copy_gram.get();
                // If ptr_copy_gram has had the empty prod_rule erased,
                // make sure the prod_rule is not reinserted when inserting pruned prod_rules
                if (grams_w_erased_empty_rule.count(&copy_gram)) {
                    internal_parser_normalizer::
                    insert_all_nempty_pruned(copy_gram, nonterminal(*w_empty_rule));
                } else {
                    internal_parser_normalizer::
                    insert_all_pruned(copy_gram, nonterminal(*w_empty_rule));
                }
            }
        }

        grams_w_empty_rule = get_grams_w_empty_rule();
    }
}

namespace cfg_parser::internal_parser_normalizer {
// __Helper for replace_reachable_unit_rules__
grammar::iterator get_unit_rule(grammar& gram) {
    return find_if(
        gram.begin(), gram.end(),
        [&](const prod_rule& rule) { return rule.is_unit(); }
    );
}

}

void parser::impl::normalizer::
replace_unit_rules(nonterminal nont) {
    auto& copy_gram = *mut_copies.at(nont);

    /* Contains nonterminal(copy_gram) and
    nonterminals in erased unit rules */
    unordered_set<nonterminal> nonts_to_keep_out;
    nonts_to_keep_out.insert(nonterminal(copy_gram));

    auto it = internal_parser_normalizer::get_unit_rule(copy_gram);
    while (it != copy_gram.end()) {
        const auto nont = it->front().as_nont();
        nonts_to_keep_out.insert(nont);

        for (const auto& rule : *nont) {
            copy_gram.insert(rule);
        }

        /* Prevents insertion of redundant rule { nonterminal(copy_gram) }
        and prevents reinsertion of unit rules already erased */
        for (const auto nt : nonts_to_keep_out) {
            copy_gram.erase({ nt });
        }

        it = internal_parser_normalizer::get_unit_rule(copy_gram);
    }
}

void parser::impl::normalizer::
replace_reachable_unit_rules() {
    const auto& gram = gram_fam.gram;
    gram.dfs_bottom_up(
        [this](nonterminal nont) { 
            replace_unit_rules(nont);
        } 
    );
}

nonterminal parser::impl::normalizer::
get_singleton_nont(terminal term) {
    const auto [it, inserted] = gram_fam.pimpl->singleton_map.emplace(
        string(1, term.get()),
        grammar{ { term } }
    );

    const auto singleton_nont = nonterminal(it->second);

    if (inserted) {
        gram_fam.pimpl->name_map_for_norm.emplace(
            singleton_nont,
            string(1, term.get())
        );
    }

    return singleton_nont;
}

/* Assume singleton_map.at(term_str[:n-1]) exists,
where term_str[:n-1] is the result of
term_str.pop_back() and !term_str.empty() */
nonterminal parser::impl::normalizer::
get_singleton_nont(const string& term_str) {
    const auto it = gram_fam.pimpl->singleton_map.find(term_str);
    if (it != gram_fam.pimpl->singleton_map.end())
        return nonterminal(it->second);

    if (term_str.size() == 1)
        return get_singleton_nont(term_str.front());

    nonterminal front_nont = get_singleton_nont(term_str.substr(0, term_str.size() - 1));
    nonterminal  back_nont = get_singleton_nont(term_str.back());

    const auto [new_it, _] = gram_fam.pimpl->singleton_map.emplace(
        term_str, grammar{ { front_nont, back_nont } }
    );

    gram_fam.pimpl->name_map_for_norm.emplace(
        nonterminal(new_it->second), term_str
    );

    return nonterminal(new_it->second);
}

size_t parser::impl::normalizer::
nont_pair_hash::operator()(const nont_pair& p) const {
    return std::hash<nonterminal>()(p.first) ^
           std::hash<nonterminal>()(p.second);
}

/* Returns a sequence of nonts, equivalent
to the rule, where rule.size() >= 2 */ 
prod_rule parser::impl::normalizer::
nont_seq_eq_of(const prod_rule& rule) {
    prod_rule nont_seq;
    string term_str;
    for (const auto& symb : rule) {
        if (symb.is_term()) {
            term_str += symb.as_term().get();
            continue;
        }

        if (!term_str.empty()) {
            nont_seq += get_singleton_nont(term_str);
            term_str.clear();
        }

        nont_seq += symb;
    }

    if (term_str.empty()) return nont_seq;
    if (nont_seq.is_empty()) {
        if (term_str.size() > 1)
            nont_seq += get_singleton_nont(term_str.substr(0, term_str.size() - 1));

        return nont_seq += get_singleton_nont(term_str.back());
    }

    return nont_seq += get_singleton_nont(term_str);
}

/* Assuming rule.size() >= 2, returns
an equivalent rule with .size() == 2 */
prod_rule parser::impl::normalizer::
nont_pair_eq_of(const prod_rule& rule) {
    const prod_rule nont_seq = nont_seq_eq_of(rule);
    auto prev = nont_seq[0].as_nont();
    for (size_t i = 1; i < nont_seq.size() - 1; i++) {
        auto curr = nont_seq[i].as_nont();
        const auto [it, _] = nont_pair_map.emplace(
            nont_pair{ prev, curr },
            std::make_unique<grammar>(
                std::initializer_list<prod_rule>{ { prev, curr } }
            )
        );

        prev = nonterminal(*it->second.get());
    }
    
    return { prev, nont_seq.back() };
}
    
void parser::impl::normalizer::
convert_reachable_rules_into_pairs() {
    for (auto& [_, ptr_copy_gram] : mut_copies) {
        vector<pair<prod_rule, prod_rule>> rules_to_convert;
        for (const auto& rule : *ptr_copy_gram) {
            if (rule.size() < 2) continue;
            if (rule.size() == 2 &&
                rule.front().is_nont() &&
                rule.back().is_nont()) continue;
            rules_to_convert.emplace_back(rule, nont_pair_eq_of(rule));
        }

        for (const auto& [old_rule, new_rule] : rules_to_convert) {
            ptr_copy_gram->erase(old_rule);
            ptr_copy_gram->insert(new_rule);
        }
    }
}

void parser::impl::normalizer::
set_norm_form() {
    if (gram_fam.gram.contains("")) {
        gram_fam.norm_form.insert("");
    }

    for (const auto& rule : *mut_copies.at(nonterminal(gram_fam.gram)).get())
        gram_fam.norm_form.insert(rule);
}

void parser::impl::normalizer::
transfer_ownership_nonts() {
    size_t serial = 1;

    const auto& gram_ptr = mut_copies.at(nonterminal(gram_fam.gram));
    for (auto& [_, ptr_copy_gram] : mut_copies) {
        if (!ptr_copy_gram->reachable_from(nonterminal(*gram_ptr)))
            continue;
        
        gram_fam.pimpl->name_map_for_norm.emplace(
            nonterminal(*ptr_copy_gram),
            std::to_string(serial++)
        );

        gram_fam.owned_grams.emplace_back(std::move(ptr_copy_gram));
    }

    for (auto& [_, ptr_copy_gram] : nont_pair_map) {
        gram_fam.pimpl->name_map_for_norm.emplace(
            nonterminal(*ptr_copy_gram),
            std::to_string(serial++)
        );

        gram_fam.owned_grams.emplace_back(std::move(ptr_copy_gram));
    }
}

void parser::impl::normalizer::
normalize() {
    verify_no_reachable_empty_nonts();
    create_deep_copy();
    replace_reachable_empty_rules();
    replace_reachable_unit_rules();
    convert_reachable_rules_into_pairs();
    set_norm_form();
    transfer_ownership_nonts();
}