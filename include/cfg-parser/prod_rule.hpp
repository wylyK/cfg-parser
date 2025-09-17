#pragma once

#include "symbol.hpp"

#include <vector>
#include <string>
#include <initializer_list>

namespace cfg_parser {

class prod_rule {

public:
    // __Special Member Functions__

    prod_rule() = default;
    prod_rule(std::initializer_list<symbol> init) : symbols(init) {}
    prod_rule(const std::string& term_seq)        : symbols(term_seq.begin(), term_seq.end()) {}
    prod_rule(const char term_seq[]);

    template <typename input_it>
    prod_rule(input_it beg, input_it end) : symbols(beg, end) {}

    template <typename symb_arg>
    explicit prod_rule(size_t count, const symb_arg& arg) : symbols(count, arg) {}

    template <typename input_it>
    prod_rule& assign(input_it beg, input_it end) { symbols.assign(beg, end); return *this; }

    template <typename symb_arg>
    prod_rule& assign(size_t count, const symb_arg& arg) { symbols.assign(count, arg); return *this; }

    template <typename symb_arg>
    prod_rule& operator=(const symb_arg& arg)                { symbols.assign(1, arg); return *this; }
    prod_rule& operator=(std::initializer_list<symbol> init) { symbols = init; return *this; }
    prod_rule& operator=(const std::string& term_seq)        { symbols.assign(term_seq.begin(), term_seq.end()); return *this; }
    prod_rule& operator=(const char term_seq[]);

    // __Iterators__

    using iterator       = std::vector<symbol>::iterator;
    using const_iterator = std::vector<symbol>::const_iterator;

    iterator begin()             { return symbols.begin(); }
    iterator   end()             { return symbols.end();   }

    const_iterator begin() const { return symbols.begin(); }
    const_iterator   end() const { return symbols.end();   }

    symbol& front()              { return symbols.front(); }
    symbol&  back()              { return symbols.back();  }

    const symbol& front() const  { return symbols.front(); }
    const symbol&  back() const  { return symbols.back();  }
 
    // __Accessors__

    symbol& at(size_t index)                     { return symbols.at(index); }
    symbol& operator[](size_t index)             { return symbols.at(index); }

    const symbol& at(size_t index) const         { return symbols.at(index); }
    const symbol& operator[](size_t index) const { return symbols.at(index); }

    // __Capcity__

    size_t   size() const { return symbols.size();  }
    bool is_empty() const { return symbols.empty(); }
    bool is_unit () const { return size() == 1 && front().is_nont(); }
    bool contains(const symbol&) const;

    // __Modifiers__

    iterator erase(iterator pos)       { return symbols.erase(pos); }
    iterator erase(const_iterator pos) { return symbols.erase(pos); }

    template <typename symb_pred>
    size_t prune_if(symb_pred&&);
    size_t prune(const symbol&);

    template <typename symb_arg>
    prod_rule& operator+=(const symb_arg& arg)  /* Append */  { symbols.emplace_back(arg);   return *this; }
    prod_rule& operator+=(const prod_rule& other)             { symbols.insert(end(), other.begin(), other.end()); return *this; }
    prod_rule& operator+=(std::initializer_list<symbol> init) { symbols.insert(end(), init.begin(), init.end()); return *this; }
    prod_rule& operator+=(const std::string& term_seq)        { symbols.insert(end(), term_seq.begin(), term_seq.end()); return *this; }
    prod_rule& operator+=(const char term_seq[]);

private:
    std::vector<symbol> symbols;

    friend bool operator==(const prod_rule&, const prod_rule&);

    friend struct std::hash<prod_rule>;
};

// __Nonmember Functions of prod_rule__

template <typename append_arg>
prod_rule operator+(prod_rule lhs, const append_arg& rhs) { return lhs += rhs; }

template <typename append_arg>
prod_rule operator+(terminal term, const append_arg& rhs) { return prod_rule(1, term) += rhs; }

template <typename append_arg>
prod_rule operator+(nonterminal nont, const append_arg& rhs) { return prod_rule(1, nont) += rhs; }

template <typename append_arg>
prod_rule operator+(const symbol& symb, const append_arg& rhs) { return prod_rule(1, symb) += rhs; }

inline prod_rule operator+(char ch, prod_rule&  rule) { return prod_rule(1, ch) += rule; }
inline prod_rule operator+(char ch,    terminal term) { return prod_rule(1, ch) += term; }
inline prod_rule operator+(char ch, nonterminal nont) { return prod_rule(1, ch) += nont; }

inline bool operator==(const prod_rule& lhs, const prod_rule& rhs) { return lhs.symbols == rhs.symbols; }
inline bool operator!=(const prod_rule& lhs, const prod_rule& rhs) { return !(lhs == rhs); }

} // End of namespace cfg_parser

namespace std {
    template<>
    struct hash<cfg_parser::prod_rule>{ 
        size_t operator()(const cfg_parser::prod_rule& rule) const;
    };
}

// __Implementation Details__

template <typename symb_pred>
size_t cfg_parser::prod_rule::prune_if(symb_pred&& pred) {
    size_t old_size = size();
    const auto new_end = std::remove_if(begin(), end(), pred);
    symbols.erase(new_end, end());
    return old_size - size();
}