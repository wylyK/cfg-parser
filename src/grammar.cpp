
#include "cfg_parser.hpp"

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stdexcept>

using std::unordered_set;
using std::unordered_map;
using std::vector;
using std::string;
using std::initializer_list;
using std::queue;
using std::unique_ptr;

using namespace cfg_parser;

class grammar::impl {

public:
    const grammar* const this_gram; // Ptr to parent grammar

    unordered_set<terminal>    terminals;
    unordered_set<nonterminal> nonterminals;
    unordered_set<prod_rule>   production_rules;

    /* Whether terminals and nonterminals aren't up
    to date with the contents of production_rules */
    bool needs_update = false;

    impl(const grammar* ptr) : this_gram(ptr) {}
    impl(const grammar* ptr, std::unordered_set<prod_rule> rules)
        : this_gram(ptr), production_rules(rules) {}

    impl(const grammar* ptr, initializer_list<prod_rule> init)
        : this_gram(ptr), production_rules(init) {}  

    /* A prod_rule rule is redundant wrt
    its grammar gram if rule == { gram }. */
    bool   is_redundant(const prod_rule&);
    void insert_members(const prod_rule&);
    void update_members();
    
    struct deep_copier {
        unordered_map<nonterminal, unique_ptr<grammar>> mut_copies;

        deep_copier(const grammar& start) { 
            mut_copies.emplace(nonterminal(start), std::make_unique<grammar>()); 
        }

        prod_rule copy(prod_rule rule) {
            for (auto& symb : rule) {
                if (symb.is_term()) continue;
                symb = nonterminal(*mut_copies.at(symb.as_nont()).get());
            }

            return rule;
        }

        // Assume mut_copies.at(curr) exists
        void operator()(nonterminal curr) {
            for (const auto nont : curr->nonterminals()) {
                mut_copies.try_emplace(nont, std::make_unique<grammar>());
            }

            for (const auto& rule : *curr) {
                mut_copies.at(curr)->insert(copy(rule));
            }
        }
    };
}; // End of class grammar::impl

bool grammar::impl::is_redundant(const prod_rule& rule) {
    return rule.is_unit() &&
           rule.front().as_nont() == nonterminal(*this_gram);
}

void grammar::impl::insert_members(const prod_rule& rule) {
    for (const auto& symb : rule) {
        if (symb.is_term()) {
            terminals.insert(symb.as_term());
            continue;
        }

        nonterminals.insert(symb.as_nont());
    }
}

void grammar::impl::update_members() {
    terminals.clear();
    nonterminals.clear();
    for (const auto& rule : production_rules) { 
        insert_members(rule); 
    }
}

grammar::grammar() : pimpl(std::make_unique<impl>(this)) {}

grammar::grammar(initializer_list<prod_rule> init)
    : pimpl(std::make_unique<impl>(this, init)) {
    for (const auto& rule : *this) {
        if (pimpl->is_redundant(rule))
            throw std::invalid_argument("Grammar cannot contain redundant rule.");
    }

    for (const auto& rule : *this) {
        pimpl->insert_members(rule);
    }
}

grammar::grammar(const grammar& other)
    : pimpl(std::make_unique<impl>(this, other.pimpl->production_rules)) {
    for (const auto& rule : *this) {
        pimpl->insert_members(rule);
    }
}

unordered_map<
    nonterminal,
    unique_ptr<grammar>
> grammar::deep_copy() const & {
    impl::deep_copier copier(*this);
    this->dfs(copier);
    return std::move(copier.mut_copies);
}

grammar::~grammar() = default;

grammar& grammar::operator=(const grammar& other) {
    pimpl->production_rules = other.pimpl->production_rules;
    pimpl->needs_update = true;
    return *this;
}

grammar& grammar::operator=(initializer_list<prod_rule> init) {
    for (const auto& rule : init) {
        if (pimpl->is_redundant(rule))
            throw std::invalid_argument("Grammar cannot contain redundant rule.");
    }

    pimpl->production_rules = init;
    pimpl->needs_update = true;
    return *this;
}

grammar::iterator grammar::begin() {
    return pimpl->production_rules.begin();
}

grammar::iterator grammar::end() {
    return pimpl->production_rules.end();
}

grammar::const_iterator grammar::begin() const {
    return pimpl->production_rules.begin();
}

grammar::const_iterator grammar::end() const {
    return pimpl->production_rules.end();
}

const unordered_set<terminal>& grammar::terminals() const {
    if (pimpl->needs_update) pimpl->update_members();
    return pimpl->terminals;
}

const unordered_set<nonterminal>& grammar::nonterminals() const {
    if (pimpl->needs_update) pimpl->update_members();
    return pimpl->nonterminals;
}

size_t grammar::size() const {
    return pimpl->production_rules.size();
}

bool grammar::is_empty() const {
    return pimpl->production_rules.empty();
}

bool grammar::contains(const prod_rule& target) const {
    return pimpl->production_rules.count(target);
}

bool grammar::insert(const prod_rule& rule) {
    if (pimpl->is_redundant(rule))
        throw std::invalid_argument("Grammar cannot contain redundant rule.");

    bool is_inserted = pimpl->production_rules.insert(rule).second;
    if (is_inserted && !pimpl->needs_update)
        pimpl->insert_members(rule);

    return is_inserted;
}

bool grammar::insert(prod_rule&& rule) {
    if (pimpl->is_redundant(rule))
        throw std::invalid_argument("Grammar cannot contain redundant rule.");

    const auto [it, is_inserted] = pimpl->production_rules.insert(std::move(rule));
    if (is_inserted && !pimpl->needs_update)
        pimpl->insert_members(*it);

    return is_inserted;
}

bool grammar::erase(const prod_rule& rule) {
    size_t num_erased = pimpl->production_rules.erase(rule);
    if (num_erased > 0)
        pimpl->needs_update = true;

    return num_erased > 0;
}

void grammar::clear() { 
    pimpl->production_rules.clear();
    pimpl->nonterminals.clear();
    pimpl->terminals.clear();
    pimpl->needs_update = false;
}

grammar& grammar::operator+=(const grammar& gram) {
    this->insert({ nonterminal(gram) });
    return *this;
}

grammar& grammar::operator+=(initializer_list<prod_rule> init) {
    for (const auto& rule : init) {
        if (!pimpl->is_redundant(rule))
             pimpl->production_rules.insert(rule);
    }

    return *this;
}

grammar& grammar::operator*=(const grammar& gram) {
    queue<prod_rule> rules;
    for (const auto& rule : pimpl->production_rules) {
        const prod_rule new_rule = rule + nonterminal(gram);
        if (!pimpl->is_redundant(new_rule))
            rules.push(new_rule);
    }
    pimpl->production_rules.clear();
    while (!rules.empty()) {
        pimpl->production_rules.insert(
            pimpl->production_rules.end(),
            rules.front()
        );
        rules.pop();
    }

    pimpl->nonterminals.insert(nonterminal(gram));
    return *this;
}

grammar& grammar::operator*=(initializer_list<prod_rule> init) {
    vector<prod_rule> rules;
    for (const auto& lhs : pimpl->production_rules) {
        for (const auto& rhs : init) {
            const prod_rule rule = lhs + rhs;
            if (!pimpl->is_redundant(rule))
                 rules.push_back(rule);
        }
    }
    pimpl->production_rules.clear();
    for (const auto& rule : rules) {
        pimpl->production_rules.insert(rule);
    }
    return *this;
}

namespace internal_grammar {

struct reachability_decider {
    unordered_set<nonterminal> found;
    bool already_found(nonterminal nont) {
        return !found.insert(nont).second;
    }

    bool is_reachable(nonterminal from, nonterminal to) {
        if (already_found(from)) return false;
        if (from == to) return true;
        for (const auto next : from->nonterminals()) {
            if (is_reachable(next, to)) return true;
        }
        
        return false;
    }
};

} // End of namespace internal_grammar

bool grammar::reachable_from(nonterminal nont) const {
    return internal_grammar::
    reachability_decider().is_reachable(nont, nonterminal(*this));
}