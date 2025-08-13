#pragma once

#include "cfg.hpp"

#include <set>

namespace cfg_parser {

template <typename NonterminalVisitor>
class Cfg::Traverser {
    NonterminalVisitor visit;
    std::unordered_set<Cfg::Nonterminal> found;

public:
    Traverser(const NonterminalVisitor& visitor) : visit(visitor) {}

    bool already_found(const Cfg::Nonterminal nonterminal) {
        return !found.insert(nonterminal).second;
    }

    void Dfs_recur(Nonterminal curr) {
        if (already_found(curr)) return;
        visit(curr);
        for (const auto next : curr->nonterminals()) {
            Dfs_recur(next);
        }
    }

    void Dfs_recur_defer_visit(Nonterminal curr) {
        if (already_found(curr)) return;
        for (const auto next : curr->nonterminals()) {
            Dfs_recur_defer_visit(next);
        }
        visit(curr);
    }
};

template <typename NonterminalVisitor>
void Cfg::Dfs(const NonterminalVisitor& visitor) const {
    Traverser<NonterminalVisitor> traverser(visitor);
    traverser.Dfs_recur(this);
}

template <typename NonterminalVisitor>
void Cfg::Dfs_defer(const NonterminalVisitor& visitor) const {
    Traverser<NonterminalVisitor> traverser(visitor);
    traverser.Dfs_recur_defer_visit(this);
}

}