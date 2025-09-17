#pragma once

#include "prod_rule.hpp"

#include <initializer_list>
#include <unordered_set>
#include <utility>
#include <memory>

namespace cfg_parser {

class grammar {
    
public:
    // __Special Member Functions__
    
    grammar();
    grammar(std::initializer_list<prod_rule>);
    grammar(const grammar&);
    grammar(grammar&&) = delete;
   ~grammar();

   // Creates a copy for each reachable nonterminal
    std::unordered_map<
        nonterminal,
        std::unique_ptr<grammar>
    > deep_copy() const &;

    std::unordered_map<
        nonterminal,
        std::unique_ptr<grammar>
    > deep_copy() && = delete;

    grammar& operator=(const grammar&);
    grammar& operator=(grammar&&) = delete;
    grammar& operator=(std::initializer_list<prod_rule>);

    // __Iterators__

    using iterator       = std::unordered_set<prod_rule>::iterator;
    using const_iterator = std::unordered_set<prod_rule>::const_iterator;

    iterator begin();
    iterator   end();

    const_iterator begin() const;
    const_iterator   end() const;

    // __Accessors__

    const std::unordered_set<   terminal>&    terminals() const;
    const std::unordered_set<nonterminal>& nonterminals() const;

    // __Capacity__

    size_t   size() const;
    bool is_empty() const;
    bool contains(const prod_rule&) const;

    // __Modifiers__

    bool insert(const prod_rule&);
    bool insert(prod_rule&&);
    bool erase(const prod_rule&);
    void clear();

    // Union assignment
    grammar& operator+=(const grammar&);
    grammar& operator+=(std::initializer_list<prod_rule>);

    // Concatenation assignment
    grammar& operator*=(const grammar&);
    grammar& operator*=(std::initializer_list<prod_rule>);

    // __Traversers__

    /* Traverses depth-first the space of nonterminals where
    A has an arc to B iff A.nonterminals() contains B */
    template <typename nont_visitor>
    void dfs(nont_visitor&&) const;

    /* Defers visiting this nonterminal until after calling
    dfs_bottom_up on everything in this->nonterminals() */
    template <typename nont_visitor>
    void dfs_bottom_up(nont_visitor&&) const;

    /* A nonterminal B is reachable from
    A if B can be visited via A->dfs */
    bool reachable_from(nonterminal) const;

    template <typename nont_pred>
    const grammar* find_if(nont_pred&&) const;

    template <typename nont_pred>
    const grammar* find_bottom_up_if(nont_pred&&) const;
    
private:
    template <typename nont_visitor>
    class traverser;
    
    class impl;
    std::unique_ptr<impl> pimpl;
};

// __Nonmember Functions of grammar__

inline grammar operator+(grammar lhs, const grammar& rhs) /* Union */       { return lhs += rhs; }
inline grammar operator+(grammar lhs, std::initializer_list<prod_rule> rhs) { return lhs += rhs; }
inline grammar operator*(grammar lhs, const grammar& rhs) /* Concat */      { return lhs *= rhs; }
inline grammar operator*(grammar lhs, std::initializer_list<prod_rule> rhs) { return lhs *= rhs; }

// __Implementation Details__ 

template <typename callable>
class grammar::traverser {
    callable& func; // visitor/predicate
    std::unordered_set<nonterminal> found;

public:
    traverser(callable& f) : func(f) {}

    bool already_found(nonterminal nont) {
        return !found.insert(nont).second;
    }

    void dfs(nonterminal curr) {
        if (already_found(curr)) return;
        func(curr); // visit
        for (const auto next : curr->nonterminals()) {
            dfs(next);
        }
    }

    void dfs_bottom_up(nonterminal curr) {
        if (already_found(curr)) return;
        for (const auto next : curr->nonterminals()) {
            dfs_bottom_up(next);
        }

        func(curr);
    }

    const grammar* find_if(nonterminal curr) {
        if (already_found(curr)) return nullptr;
        if (func(curr)) return &*curr;
        const grammar* gram_ptr;
        for (const auto next : curr->nonterminals()) {
            gram_ptr = find_if(next);
            if (gram_ptr != nullptr) return gram_ptr;
        }
        
        return nullptr;
    }

    const grammar* find_bottom_up_if(nonterminal curr) {
        if (already_found(curr)) return nullptr;
        const grammar* gram_ptr;
        for (const auto next : curr->nonterminals()) {
            gram_ptr = find_bottom_up_if(next);
            if (gram_ptr != nullptr) return gram_ptr;
        }
         
        if (func(curr)) return &*curr;
        return nullptr;
    }
};

template <typename nont_visitor>
void grammar::dfs(nont_visitor&& visitor) const {
    traverser<nont_visitor> trav(visitor);
    trav.dfs(nonterminal(*this));
}

template <typename nont_visitor>
void grammar::dfs_bottom_up(nont_visitor&& visitor) const {
    traverser<nont_visitor> trav(visitor);
    trav.dfs_bottom_up(nonterminal(*this));
}

template <typename nont_pred>
const grammar* grammar::find_if(nont_pred&& pred) const {
    traverser<nont_pred> trav(pred);
    return trav.find_if(nonterminal(*this));
}

template <typename nont_pred>
const grammar* grammar::find_bottom_up_if(nont_pred&& pred) const {
    traverser<nont_pred> trav(pred);
    return trav.find_bottom_up_if(nonterminal(*this));
}

} // End of namespace cfg_parser