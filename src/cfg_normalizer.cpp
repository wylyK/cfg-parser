#include "cfg_normal_form.hpp"
#include "cfg_normalizer.hpp"
#include "cfg_helpers.hpp"

#include <algorithm>
#include <utility>
#include <stdexcept>

using std::unordered_map;
using std::set;
using std::vector;
using std::string;
using std::pair;

using namespace cfg_parser;

CfgParser::Impl::Normalizer::Normalizer(Cnf& norm)
    : norm(norm)
{
    mut_copy.emplace(norm.nonterminal_to_normalize, new Cfg{});
}

void CfgParser::Impl::Normalizer::
verify_no_reachable_empty_nonterminals() {
    norm.nonterminal_to_normalize->Dfs(
        [&](Cfg::Nonterminal nonterminal) {
            if (nonterminal->is_empty())
                throw std::logic_error(norm.pimpl->name_map[nonterminal] + " is empty.");
        }
    );
}

Cfg::Rule CfgParser::Impl::Normalizer::
nonterminals_replaced_w_copy(const Cfg::Rule& rule) {
    Cfg::Rule resultant_rule;
    for (const auto& symbol : rule) {
        resultant_rule += is_nonterminal(symbol)  ?
           mut_copy[as_nonterminal(symbol)] : symbol;
    }
    return resultant_rule;
}
    
void CfgParser::Impl::Normalizer::
create_copy(Cfg::Nonterminal curr) {
    for (const auto next : curr->nonterminals()) {
        mut_copy.emplace(next, Cfg{});
    }

    // Assume mut_copy[curr] exists
    for (const auto& rule : curr->rules()) {
        mut_copy[curr]->insert(nonterminals_replaced_w_copy(rule));
    }
}

void CfgParser::Impl::Normalizer::
create_deep_copy() {
    norm.nonterminal_to_normalize->Dfs(
        [this](Cfg::Nonterminal nonterminal) {
            create_copy(nonterminal);
        }
    );
}

vector<Cfg*> CfgParser::Impl::Normalizer::
get_nonterminals_w_empty_rule() {
    vector<Cfg*> result;
    for (auto [_, copy_nonterminal] : mut_copy) {
        if (!copy_nonterminal->contains({})) continue;
        result.push_back(copy_nonterminal);
    }
    return result;
}

void insert_pruned_rules(
    Cfg& copy_grammar,
    Cfg::Nonterminal nonterminal
) {
    vector<Cfg::Rule> pruned_rules_to_insert;

    for (const auto& rule : copy_grammar.rules()) {
        Cfg::Rule pruned_rule = rule;
        if (pruned_rule.prune(nonterminal)) {
            pruned_rules_to_insert.push_back(pruned_rule);
        }
    }

    for (const auto& rule : pruned_rules_to_insert) {
        copy_grammar.insert(rule);
    }
}

void insert_nonempty_pruned_rules(
    Cfg& copy_grammar,
    Cfg::Nonterminal nonterminal
) {
    vector<Cfg::Rule> pruned_rules_to_insert;

    for (const auto& rule : copy_grammar.rules()) {
        Cfg::Rule pruned_rule = rule;
        if (pruned_rule.prune(nonterminal) &&
           !pruned_rule.is_empty()) {
            pruned_rules_to_insert.push_back(pruned_rule);
        }
    }

    for (const auto& rule : pruned_rules_to_insert) {
        copy_grammar.insert(rule);
    }
}

void CfgParser::Impl::Normalizer::
replace_reachable_empty_rules() {
    vector<Cfg*> nonterminals_w_empty_rule;
    vector<Cfg::Nonterminal> nonterminals_w_erased_empty_rule;
    while (!(nonterminals_w_empty_rule = get_nonterminals_w_empty_rule()).empty()) {
        for (const auto nonterminal : nonterminals_w_empty_rule) {
            nonterminal->erase({});
            nonterminals_w_erased_empty_rule.push_back(nonterminal);

            for (auto& [_, copy_nonterminal] : mut_copy) {

                // If copy_nonterminal has had the empty rule erased,
                // make sure the rule is not reinserted when inserting pruned rules
                if (std::find(
                    nonterminals_w_erased_empty_rule.begin(),
                    nonterminals_w_erased_empty_rule.end(),
                    copy_nonterminal
                ) == nonterminals_w_erased_empty_rule.end()) {
                    insert_pruned_rules(*copy_nonterminal, nonterminal);
                    continue;
                }
                insert_nonempty_pruned_rules(*copy_nonterminal, nonterminal);
            }
        }
    }
}

void CfgParser::Impl::Normalizer::
replace_unit_rules(Cfg::Nonterminal curr) {
    auto& copy_grammar = *mut_copy[curr];

    // Contains current grammar and nonterminals in erased unit rules 
    std::unordered_set<Cfg::Nonterminal> nonterminals_to_keep_out;
    nonterminals_to_keep_out.insert(&copy_grammar);

    set<Cfg::Rule>::iterator it;

    // Loop until no unit rules exists
    while ((it = std::find_if(
            copy_grammar.rules().begin(),
            copy_grammar.rules().end(),
            [](const Cfg::Rule& rule) { return rule.is_unit(); }
            )) != copy_grammar.rules().end()
        ) {
        const auto nonterminal = as_nonterminal(it->front());
        nonterminals_to_keep_out.insert(nonterminal);

        for (const auto& rule : nonterminal->rules()) {
            copy_grammar.insert(rule);
        }

        // Prevents insertion of redundant rule and
        // prevents reinsertion of erased unit rules
        for (const auto nt : nonterminals_to_keep_out) {
            copy_grammar.erase({ nt });
        }
    }
}

void CfgParser::Impl::Normalizer::
replace_reachable_unit_rules() {
    mut_copy[norm.nonterminal_to_normalize]->Dfs_defer(
        [this](Cfg::Nonterminal nonterminal) {
            replace_unit_rules(nonterminal);
        }
    );
}

Cfg::Nonterminal CfgParser::Impl::Normalizer::
get_singleton_nonterminal(Cfg::Terminal terminal) {
    const auto [it, inserted] = norm.pimpl->singleton_map.emplace(
        string(1, terminal),
        Cfg{ { terminal } });

    Cfg::Nonterminal singleton_nonterminal = &it->second;

    if (inserted) {
        norm.pimpl->name_map_for_norm.emplace(
            singleton_nonterminal,
            string(1, terminal)
        );
    }

    return singleton_nonterminal;
}

// Assume pimpl->singleton_map[terminal_str[:n-1]] exists,
// where terminal_str[:n-1] is the result of terminal_str.pop_back()
Cfg::Nonterminal CfgParser::Impl::Normalizer::
get_singleton_nonterminal(string terminal_str) {
    auto it = norm.pimpl->singleton_map.find(terminal_str);
    
    if (it != norm.pimpl->singleton_map.end()) return &it->second;

    Cfg::Nonterminal front_nonterminal = get_singleton_nonterminal(terminal_str.substr(0, terminal_str.size() - 1));
    Cfg::Nonterminal  back_nonterminal = get_singleton_nonterminal(terminal_str.back());

    const auto [it, _] = norm.pimpl->singleton_map.emplace(
        terminal_str,
        Cfg{ { front_nonterminal, back_nonterminal } }
    );

    norm.pimpl->name_map_for_norm.emplace(&it->second, terminal_str);

    return &it->second;
}

size_t CfgParser::Impl::Normalizer::NonterminalPairHash::
operator()(const NonterminalPair& pair) const {
    return std::hash<Cfg::Nonterminal>()(pair.first) ^
           std::hash<Cfg::Nonterminal>()(pair.second);
}

// Assuming rule.size() >= 3,
// returns an equivalent copy of rule with .size() == 2
Cfg::Rule CfgParser::Impl::Normalizer::
nonterminal_pair_equiv_of(const Cfg::Rule& rule) {
    Cfg::Rule result;
    string terminal_str;
    for (const auto& symbol : rule) {
        if (is_terminal) {
            terminal_str += as_terminal(symbol);
            continue;
        }

        if (!terminal_str.empty()) {
            result += get_singleton_nonterminal(terminal_str);
            terminal_str.clear();
        }

        result += symbol;
    }

    Cfg::Nonterminal prev = as_nonterminal(result.front());
    for (size_t i = 1; i < result.size() - 1; i++) {
        Cfg::Nonterminal curr = as_nonterminal(result[i]);
        const auto [it, inserted] = nonterminal_pair_map.emplace(
            NonterminalPair{ prev, curr },
            new Cfg{ { prev, curr }}
        );

        prev = it->second;
    }
    return { prev, result.back() };
}
    
void CfgParser::Impl::Normalizer::
convert_reachable_rules_into_pairs() {
    for (auto [_, copy_nonterminal] : mut_copy) {
        vector<pair<Cfg::Rule, Cfg::Rule>> rules_to_convert;
        for (const auto& rule : copy_nonterminal->rules()) {
            if (rule.size() < 2) continue;
            rules_to_convert.emplace_back(rule, nonterminal_pair_equiv_of(rule));
        }
        for (const auto& [old_rule, new_rule] : rules_to_convert) {
            copy_nonterminal->erase(old_rule);
            copy_nonterminal->insert(new_rule);
        }
    }
}

void CfgParser::Impl::Normalizer::
transfer_ownership_of_nonterminals() {
    unsigned serial = 1;

    const auto& head = mut_copy[norm.nonterminal_to_normalize];
    for (const auto [_, copy_nonterminal] : mut_copy) {
        if (!copy_nonterminal->reachable_from(head)) {
            delete copy_nonterminal;
            continue;
        }
        
        norm.owned_nonterminals.push_back(copy_nonterminal);
        norm.pimpl->name_map_for_norm.emplace(
            copy_nonterminal,
            std::to_string(serial++)
        );
    }

    for (const auto [_, nonterminal] : nonterminal_pair_map) {
        norm.owned_nonterminals.push_back(nonterminal);
        norm.pimpl->name_map_for_norm.emplace(
            nonterminal,
            std::to_string(serial++)
        );
    }
}

void CfgParser::Impl::Normalizer::
set_norm_form() {
    if (norm.nonterminal_to_normalize->contains({})) {
        norm.norm_form.insert({});
    }

    for (const auto& rule : mut_copy[norm.nonterminal_to_normalize]->rules())
        norm.norm_form.insert(rule);
}

void CfgParser::Impl::Normalizer::
normalize() {
    verify_no_reachable_empty_nonterminals();
    create_deep_copy();
    replace_reachable_empty_rules();
    replace_reachable_unit_rules();
    convert_reachable_rules_into_pairs();
    transfer_ownership_of_nonterminals();
    set_norm_form();
}