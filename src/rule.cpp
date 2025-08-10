#include "rule.hpp"
#include "cfg_helpers.hpp"

#include <utility>
#include <iostream>
#include <algorithm>

using std::string;
using std::cout;
using std::endl;

namespace cfg_parser {

Cfg::Rule::Rule(const Symbol& symbol) { symbols_.assign(1, symbol); }

Cfg::Rule::Rule(const std::string& terminal_str) {
    symbols_.assign(
        terminal_str.begin(),
        terminal_str.end()
    );
}

Cfg::Rule& Cfg::Rule::operator=(const Symbol& symbol) {
    symbols_.assign(1, symbol);
    return *this;
}

Cfg::Rule& Cfg::Rule::operator=(const std::string& terminal_str) {
    symbols_.assign(
        terminal_str.begin(),
        terminal_str.end()
    );
    return *this;
}

Cfg::Rule& Cfg::Rule::operator+=(const Symbol& symbol) {
    symbols_.push_back(symbol);
    return *this;
}

Cfg::Rule& Cfg::Rule::operator+=(const std::string& terminal_str) {
    symbols_.insert(
        symbols_.end(),
        terminal_str.begin(),
        terminal_str.end()
    );
    return *this;
}

Cfg::Rule& Cfg::Rule::operator+=(const Cfg::Rule& other) {
    symbols_.insert(
        symbols_.end(),
        other.symbols_.begin(),
        other.symbols_.end()
    );
    return *this;
}

bool Cfg::Rule::is_unit() const {
    return symbols_.size() == 1 && is_nonterminal(symbols_.front());
}

bool Cfg::Rule::contains(const Cfg::Symbol& target_symbol) const {
    return std::find(
        symbols_.begin(),
        symbols_.end(),
        target_symbol) != symbols_.end();
}

size_t Cfg::Rule::prune(const Symbol& symbol) {
    const size_t old_size = size();
    const auto new_end = std::remove(
        symbols_.begin(),
        symbols_.end(),
        symbol);
    symbols_.erase(new_end, symbols_.end());
    return old_size - size();
}

Cfg::Rule operator+(
    Cfg::Rule rule_lhs,
    const Cfg::Rule& rule_rhs
) { 
    return rule_lhs += rule_rhs;
}

Cfg::Rule operator+(
    Cfg::Rule rule,
    const Cfg::Symbol& symbol
) { 
    return rule += symbol;
}

Cfg::Rule operator+(
    const Cfg::Symbol& symbol,
    const Cfg::Rule& rule
) {
    Cfg::Rule result{ symbol };
    return result += rule;
}

Cfg::Rule operator+( 
    Cfg::Rule rule,
    const std::string& terminal_str
) {
    return rule += terminal_str;
}

Cfg::Rule operator+(
    const std::string& terminal_str,
    const Cfg::Rule& rule
) {
    Cfg::Rule result(terminal_str);
    return result += rule;
}

bool compare(
    const Cfg::Symbol& symbol_lhs,
    const Cfg::Symbol& symbol_rhs
) {
    if (is_terminal(symbol_lhs)) {
        if (is_nonterminal(symbol_rhs)) return true;

        return as_terminal(symbol_lhs) <
               as_terminal(symbol_rhs);
    }

    if (is_terminal(symbol_rhs)) return true;

    return as_nonterminal(symbol_lhs) <
           as_nonterminal(symbol_rhs);
}

bool operator<(
    const Cfg::Rule& rule_lhs,
    const Cfg::Rule& rule_rhs
) {
    size_t num_nt_lhs = 0;
    size_t num_nt_rhs = 0;
    
    for (const auto& symbol : rule_lhs) {
        if (is_nonterminal(symbol)) num_nt_lhs++;
    }
    
    for (const auto& symbol : rule_rhs) {
        if (is_nonterminal(symbol)) num_nt_lhs++;
    }

    if (num_nt_lhs != num_nt_rhs)
        return num_nt_lhs < num_nt_rhs;

    if (rule_lhs.size() != rule_rhs.size())
        return rule_lhs.size() < rule_rhs.size();

    for (size_t i = 0; i < rule_lhs.size(); i++) {
        if (!compare(rule_lhs.at(i), rule_rhs.at(i)))
            return true;
    }

    return false;
};

bool operator==(
    const Cfg::Rule& rule_lhs,
    const Cfg::Rule& rule_rhs
) {
    return rule_lhs.symbols() == rule_rhs.symbols();
}

bool operator!=(
    const Cfg::Rule& rule_lhs,
    const Cfg::Rule& rule_rhs
) {
    return !(rule_lhs == rule_rhs);
}

bool operator<=(
    const Cfg::Rule& rule_lhs,
    const Cfg::Rule& rule_rhs
) {
    return (rule_lhs < rule_rhs) || (rule_lhs == rule_rhs);
}