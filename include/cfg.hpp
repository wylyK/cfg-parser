#pragma once

#include <set>
#include <unordered_set>
#include <memory>
#include <variant>
#include <utility>
#include <functional>

namespace cfg_parser {

class Cfg {
    
public:
    using Terminal = char;
    using Nonterminal = const Cfg*;
    using Symbol = std::variant<Terminal, Nonterminal>;
    
    class Rule;

    Cfg() = default;
    Cfg(std::initializer_list<Rule> rules);
    Cfg(const Cfg& other_grammar);

    Cfg& operator=(const Cfg& other_grammar);
    Cfg& operator+=(const Cfg& grammar);
    Cfg& operator+=(Cfg&& grammar);
    Cfg& operator*=(const Cfg& grammar);
    Cfg& operator*=(Cfg&& grammar);

    const std::unordered_set<Terminal>& terminals() const;
    const std::unordered_set<Nonterminal>& nonterminals() const;
    const std::set<Rule>& rules() const;

    size_t   size() const;
    bool is_empty() const;
    bool contains(const Rule& rule) const;

    bool  insert(const Rule& new_rule);
    size_t erase(const Rule& old_rule);
    void   clear();

    // Traverses depth-first the space of nonterminals where
    // A has an arc to B iff A.nonterminals() contains B
    template <typename NonterminalVisitor>
    void Dfs(const NonterminalVisitor& visitor) const;

    template <typename NonterminalVisitor>
    void Dfs_defer(const NonterminalVisitor& visitor) const;

    // A nonterminal A is reachable from another nonterminal B
    // if A can be visited via B->Dfs
    bool reachable_from(Nonterminal nonterminal) const;
    
private:
    template <typename NonterminalVisitor>
    class Traverser;
    
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

Cfg operator+(Cfg grammar_lhs, const Cfg& grammar_rhs);
Cfg operator+(Cfg grammar_lhs, Cfg&& grammar_rhs);
Cfg operator*(Cfg grammar_lhs, const Cfg& grammar_rhs);
Cfg operator*(Cfg grammar_lhs, Cfg&& grammar_rhs);

}

#include "cfg.tpp"