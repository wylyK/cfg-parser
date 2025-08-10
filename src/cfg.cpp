#include "cfg.hpp"
#include "cfg_helpers.hpp"
#include "rule.hpp"

#include <map>
#include <queue>
#include <fstream>
#include <iostream>
#include <stdexcept>

using std::map;
using std::set;
using std::unordered_set;
using std::vector;
using std::string;
using std::queue;
using std::function;
using std::cout;
using std::endl;

namespace cfg_parser {

class Cfg::Impl {

public:
    const Cfg* const this_grammar;

    unordered_set<Terminal> terminals;
    unordered_set<Nonterminal> nonterminals;

    bool needs_reset = false;

    set<Rule> rules;

    Impl(const Cfg* this_grammar) : this_grammar(this_grammar) {}

    bool is_redundant(const Rule& rule);
    void verify(const Rule& new_rule);
    void insert_members(const Rule& new_rule);
    void reset_members();
};

bool Cfg::Impl::is_redundant(const Rule& rule) {
    return rule.is_unit() && as_nonterminal(rule.front()) == this_grammar;
}

void Cfg::Impl::verify(const Cfg::Rule& new_rule) {
    if (new_rule.contains(nullptr))
        throw std::invalid_argument("Grammar cannot contain nullptr nonterminals.");

    if (is_redundant(new_rule))
        throw std::invalid_argument ("Grammar cannot contain redundant rule.");
}

void Cfg::Impl::insert_members(const Cfg::Rule& new_rule) {
    for (const auto& symbol : new_rule) {
        if (is_terminal(symbol)) {
            terminals.insert(as_terminal(symbol));
            continue;
        }
        nonterminals.insert(as_nonterminal(symbol));
    }
}

void Cfg::Impl::reset_members() {
    terminals.clear();
    nonterminals.clear();
    for (const auto& rule : rules) {
        insert_members(rule);
    }
}

// Cfg Constructor
Cfg::Cfg(std::initializer_list<Rule> rules) {
    pimpl = std::make_unique<Impl>(this);

    for (const auto& rule : rules) {
        pimpl->verify(rule);
        pimpl->rules.insert(rule);
    }

    for (const auto& rule : pimpl->rules) {
        pimpl->insert_members(rule);
    }
}

// Cfg Copy Constructor
Cfg::Cfg(const Cfg& other_grammar) {
    auto pimpl = std::make_unique<Impl>(&other_grammar);
    pimpl->rules = other_grammar.pimpl->rules;
    pimpl->needs_reset = true;
}

Cfg& Cfg::operator=(const Cfg& grammar) {
    *this = Cfg(grammar);
    return *this;
}

// Unions this grammar with the lvalue argument
Cfg& Cfg::operator+=(const Cfg& grammar) {
    this->insert({ &grammar });
    return *this;
}

// Unions this grammar with the rvalue argument
Cfg& Cfg::operator+=(Cfg&& grammar) {
    for (const auto& rule : grammar.rules()) {
        if (!pimpl->is_redundant(rule))
             pimpl->rules.insert(rule);
    }
    return *this;
}

// Concatenates this grammar with the lvalue arguement
Cfg& Cfg::operator*=(const Cfg& grammar) {
    queue<Rule> new_rules;
    for (const auto& rule : pimpl->rules) {
        const Rule new_rule = rule + &grammar;
        if (!pimpl->is_redundant(new_rule))
            new_rules.push(new_rule);
    }
    pimpl->rules.clear();
    while (!new_rules.empty()) {
        pimpl->rules.insert(
            pimpl->rules.end(),
            new_rules.front()
        );
        new_rules.pop();
    }
    pimpl->nonterminals.insert(&grammar);
    return *this;
}

// Concatenates this grammar with the rvalue arguement
Cfg& Cfg::operator*=(Cfg&& grammar) {
    vector<Rule> new_rules;
    for (const auto& rule_lhs : pimpl->rules) {
        for (const auto& rule_rhs : grammar.rules()) {
            const Rule new_rule = rule_lhs + rule_rhs;
            if (!pimpl->is_redundant(new_rule))
                new_rules.push_back(new_rule);
        }
    }
    pimpl->rules.clear();
    for (const auto& new_rule : new_rules) {
        pimpl->rules.insert(new_rule);
    }
    return *this;
}

const std::unordered_set<Cfg::Terminal>& Cfg::terminals() const {
    if (pimpl->needs_reset) pimpl->reset_members();
    return pimpl->terminals;
}

const std::unordered_set<Cfg::Nonterminal>& Cfg::nonterminals() const {
    if (pimpl->needs_reset) pimpl->reset_members();
    return pimpl->nonterminals;
}

const std::set<Cfg::Rule>& Cfg::rules() const {
    return pimpl->rules;
}

size_t Cfg::size() const {
    return pimpl->rules.size();
}

bool Cfg::is_empty() const {
    return pimpl->rules.empty();
}

bool Cfg::contains(const Rule& rule) const {
    return pimpl->rules.count(rule);
}

bool Cfg::insert(const Cfg::Rule& new_rule) {
    pimpl->verify(new_rule);
    bool is_inserted = pimpl->rules.insert(new_rule).second;
    if (is_inserted && !pimpl->needs_reset)
        pimpl->insert_members(new_rule);
    return is_inserted;
}

size_t Cfg::erase(const Cfg::Rule& old_rule) {
    size_t num_erased = pimpl->rules.erase(old_rule);
    if (num_erased > 0)
        pimpl->needs_reset = true;
    return num_erased;
}

void Cfg::clear() { 
    pimpl->rules.clear();
    pimpl->nonterminals.clear();
    pimpl->terminals.clear();
}

bool reachability(
    Cfg::Nonterminal from,
    Cfg::Nonterminal to,
    unordered_set<Cfg::Nonterminal>& visited
) {
    if (from == to) return true;
    visited.insert(from);

    for (const auto next : from->nonterminals()) {
        if (visited.count(next)) continue;
        if (reachability(next, to, visited)) return true;
    }

    return false;
}

bool Cfg::reachable_from(Nonterminal nonterminal) const {
    unordered_set<Nonterminal> visited;
    return reachability(nonterminal, this, visited);
}

Cfg operator+(Cfg grammar_lhs, const Cfg& grammar_rhs) {
    return grammar_lhs += grammar_rhs;
}

Cfg operator+(Cfg grammar_lhs, Cfg&& grammar_rhs) {
    return grammar_lhs += grammar_rhs;
}

Cfg operator*(Cfg grammar_lhs, const Cfg& grammar_rhs) {
    return grammar_lhs *= grammar_rhs;
}

Cfg operator*(Cfg grammar_lhs, Cfg&& grammar_rhs) {
    return grammar_lhs *= grammar_rhs;
}

}