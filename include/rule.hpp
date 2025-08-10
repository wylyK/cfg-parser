#pragma once

#include "cfg.hpp"

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <algorithm>

namespace cfg_parser {

class Cfg::Rule {

public:
    Rule() = default;
    Rule(const Symbol& symbol);
    Rule(const std::string& terminal_str);
    Rule(std::initializer_list<Symbol> symbols) : symbols_(std::move(symbols)) {}
    Rule(const Rule& other) : symbols_(other.symbols_) {}

    Rule& operator= (const Symbol& symbol);
    Rule& operator= (const std::string& terminal_str);
    Rule& operator+=(const Symbol& symbol);
    Rule& operator+=(const std::string& terminal_str);
    Rule& operator+=(const Rule& other);

    Symbol& front() { return symbols_.front(); }
    Symbol&  back() { return symbols_.back();  }

    const Symbol& front() const { return symbols_.front(); }
    const Symbol&  back() const { return symbols_.back();  }

    Symbol& at(size_t index) { return symbols_.at(index); }
    Symbol& operator[](size_t index) { return symbols_[index]; }

    const Symbol& at(size_t index) const { return symbols_.at(index); }
    const Symbol& operator[](size_t index) const { return symbols_[index]; }

    using iterator = std::vector<Symbol>::iterator;
    using const_iterator = std::vector<Symbol>::const_iterator;

    iterator begin() { return symbols_.begin(); }
    iterator   end() { return symbols_.end();   }

    const_iterator begin() const { return symbols_.begin(); }
    const_iterator   end() const { return symbols_.end();   }
    
    const std::vector<Symbol>& symbols() const { return symbols_; }

    size_t   size() const { return symbols_.size();  }
    bool is_empty() const { return symbols_.empty(); }
    bool is_unit () const;
    bool contains(const Symbol& target_symbol) const;

    size_t prune(const Symbol& symbol);

    template <typename SymbolPredicate>
    size_t prune_if(SymbolPredicate predicate) {
        size_t old_size = size();
        const auto new_end = std::remove_if(
            symbols_.begin(),
            symbols_.end(),
            predicate
        );
        symbols.erase(new_end, symbols_.end());
        return old_size - size();
    }

private:
    std::vector<Symbol> symbols_;
};

Cfg::Rule operator+(Cfg::Rule rule_lhs, const Cfg::Rule& rule_rhs);
Cfg::Rule operator+(Cfg::Rule rule, const Cfg::Symbol& symbol);
Cfg::Rule operator+(const Cfg::Symbol& symbol, const Cfg::Rule& rule);
Cfg::Rule operator+(Cfg::Rule rule, const std::string& terminal_str);
Cfg::Rule operator+(const std::string& terminal_str, const Cfg::Rule& rule);

bool operator< (const Cfg::Rule& rule_lhs, const Cfg::Rule& rule_rhs);
bool operator==(const Cfg::Rule& rule_lhs, const Cfg::Rule& rule_rhs);
bool operator!=(const Cfg::Rule& rule_lhs, const Cfg::Rule& rule_rhs);
bool operator<=(const Cfg::Rule& rule_lhs, const Cfg::Rule& rule_rhs);

}