#pragma once

#include <variant>
#include <iostream>
#include <stdexcept>

namespace cfg_parser {

class grammar;

class terminal {

public:
    constexpr terminal(char a) : value(a) { if (a < ' ' || a > '~') throw std::invalid_argument("Character must be printable.");  }
    terminal(const terminal& term) = default;

    char get() const { return value; }

    friend bool  operator<(terminal lhs, terminal rhs) { return lhs.value <  rhs.value; }
    friend bool  operator>(terminal lhs, terminal rhs) { return lhs.value >  rhs.value; }
    friend bool operator==(terminal lhs, terminal rhs) { return lhs.value == rhs.value; }
    friend bool operator!=(terminal lhs, terminal rhs) { return lhs.value != rhs.value; }
    friend bool operator<=(terminal lhs, terminal rhs) { return lhs.value <= rhs.value; }
    friend bool operator>=(terminal lhs, terminal rhs) { return lhs.value >= rhs.value; }

    friend std::ostream& operator<<(std::ostream& os, terminal term) { return os << term.value; }
    friend struct std::hash<terminal>;

private:
    char value; // Must be printable
};

class nonterminal {

public:
    explicit nonterminal(const grammar& to_ref) : ptr(&to_ref) {}
    nonterminal(grammar&&)  = delete;

    const grammar& operator*()  const { return *ptr; }
    const grammar* operator->() const { return  ptr; }

    friend bool operator==(nonterminal, nonterminal);
    friend struct std::hash<nonterminal>;

private:
    const grammar* ptr;
};

inline bool operator==(nonterminal lhs, nonterminal rhs) { return lhs.ptr == rhs.ptr; }
inline bool operator!=(nonterminal lhs, nonterminal rhs) { return !(lhs == rhs); }

class symbol {

public:
    symbol(char term)        : container(term) {}
    symbol(terminal term)    : container(term) {}
    symbol(nonterminal nont) : container(nont) {}

    symbol& operator=(char term)        { container = term; return *this; }
    symbol& operator=(terminal term)    { container = term; return *this; }
    symbol& operator=(nonterminal nont) { container = nont; return *this; }

    bool is_term()    const { return std::holds_alternative<   terminal>(container); }
    bool is_nont() const { return std::holds_alternative<nonterminal>(container); }

    terminal&    as_term()             { return std::get<   terminal>(container); }
    nonterminal& as_nont()             { return std::get<nonterminal>(container); }

    const terminal&    as_term() const { return std::get<   terminal>(container); }
    const nonterminal& as_nont() const { return std::get<nonterminal>(container); }

    friend bool operator==(const symbol&, const symbol&);
    friend struct std::hash<symbol>;

private:
    std::variant<terminal, nonterminal> container;

    friend class prod_rule;
};

inline bool operator==(const symbol& lhs, const symbol& rhs) { return lhs.container == rhs.container; }
inline bool operator!=(const symbol& lhs, const symbol& rhs) { return !(lhs == rhs); }

} // End of namespace cfg_parser

namespace std {

template<>
struct hash<cfg_parser::terminal> {
    size_t operator()(cfg_parser::terminal term) const { return hash<char>{}(term.value); }
};

template<>
struct hash<cfg_parser::nonterminal> {
    size_t operator()(cfg_parser::nonterminal nont) const { return hash<const cfg_parser::grammar*>{}(nont.ptr); }
};

template<>
struct hash<cfg_parser::symbol> {
    size_t operator()(cfg_parser::symbol symb) const {
        return std::hash<std::variant<
            cfg_parser::terminal,
            cfg_parser::nonterminal>
        >{}(symb.container);
    }
};

} // End of namespace std