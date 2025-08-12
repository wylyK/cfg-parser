#include "cfg_parser.hpp"
#include "cfg_normal_form.hpp"
#include "cfg_printer.hpp"
#include "cfg_helpers.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>

using std::unordered_set;
using std::unordered_map;
using std::string;
using std::pair;
using std::invalid_argument;

using namespace cfg_parser;

bool CfgParser::Impl::
is_foreign(const Cfg::Nonterminal nonterminal) const {
    if (!nonterminal) return false;
    return !name_map.count(nonterminal);
}

bool CfgParser::Impl::
has_foreign_nonterminal(const Cfg::Rule& rule) const {
    return std::any_of(
        rule.begin(), rule.end(),
        [this](const Cfg::Symbol& symbol) {
            return is_nonterminal(symbol) &&
                   is_foreign(as_nonterminal(symbol));
        }
    );
}

// Create a new Cfg, provided namem_map and rule set
void CfgParser::create(
    const string& name, std::initializer_list<Cfg::Rule> rules
) {
    if (name.empty())
        throw invalid_argument("Name cannot be empty.");

    if (pimpl->grammar_map.count(name)) 
        throw invalid_argument(name + " already exists.");

    for (const auto& rule : rules) {
        if (pimpl->has_foreign_nonterminal(rule))
            throw invalid_argument(
                "Grammar cannot contain foreign nonterminals."
            );
    }

    const auto [it, _] = pimpl->grammar_map.emplace(
        name, pair<Cfg, Impl::Cnf>{ rules, Impl::Cnf{ pimpl.get() } }
    );

    auto& [grammar, norm_grammar] = it->second;
    norm_grammar.set(&grammar);

    pimpl->name_map.emplace(&grammar, name);
}

Cfg::Nonterminal CfgParser::get(const string& name) {
    const auto it = pimpl->grammar_map.find(name);
    return it != pimpl->grammar_map.end() ?
                        &it->second.first : nullptr;
}

bool CfgParser::insert(const string& name, const Cfg::Rule& rule) {
    if (!pimpl->grammar_map.count(name)) 
        throw invalid_argument(name + " doesn't exist.");

    if (pimpl->has_foreign_nonterminal(rule)) {
        throw invalid_argument(
            "Grammar cannot contain foreign nonterminals."
        );
    }
    return pimpl->grammar_map[name].first.insert(rule);
}

size_t CfgParser::erase(const string& name, const Cfg::Rule& rule) {
    if (!pimpl->grammar_map.count(name))
        throw invalid_argument(name + " doesn't exist.");

    return pimpl->grammar_map[name].first.erase(rule);
}

void CfgParser::print(const string& name) {
    if (!pimpl->grammar_map.count(name))
        throw invalid_argument(name + " doesn't exist.");

    CfgPrinter printer(
        pimpl->name_map,
        { '[', ']' }
    );
    printer(&pimpl->grammar_map[name].first);
}

void CfgParser::print_norm(const string& name) {
    if (!pimpl->grammar_map.count(name))
        throw invalid_argument(name + " doesn't exist.");

        CfgPrinter printer(
        pimpl->name_map_for_norm,
        { '{', '}' }
    );
    printer(&pimpl->grammar_map[name].second.get());
}

bool CfgParser::parse(const string& name, const string& word) {
    if (!pimpl->grammar_map.count(name))
        throw invalid_argument(name + " doesn't exist.");

    auto& normalized_grammar = pimpl->grammar_map[name].second.get();

    if (word.empty()) return normalized_grammar.contains({});

    if (word.size() == 1) {
        // Find terminal in normalized_grammar equal to word.front()

        // This is more efficient than iterating through normalized_grammar.terminals()
        // because Cfg::Impl::terminals() might involks Cfg::Impl::reset_members()
        for (const auto& rule : normalized_grammar.rules()) {
            if (rule.size() > 1) return false; // if rule isn't a single terminal
            if (as_terminal(rule.front()) == word.front())
                return true;
        }
    }

    for (size_t left_len = 1; left_len < word.size(); left_len++) {
        if (
            parse(name, word.substr(0, left_len)) &&
            parse(name, word.substr(left_len))
        )
            return true;
    }
}

void CfgParser::parse_file(const string& name, const string& file_name) {
    std::ifstream word_list("file_name");

    if (!word_list) {
        throw invalid_argument("Could not open file!");
    }

    string word;
    while (std::getline(word_list, word)) {
        std::cout << name + (parse(name, word) ? " accepts " : " rejects ") + word << std::endl;
    }
}