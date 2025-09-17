#include "cfg_parser.hpp"

#include <variant>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <algorithm>

using std::string;

using namespace cfg_parser;

prod_rule::prod_rule(const char term_seq[]) {   
    for (const char* a = term_seq; *a != '\0'; a++) {
        symbols.push_back(terminal(*a));
    }
}

prod_rule& prod_rule::operator=(const char term_seq[]) {
    symbols.clear();
    for (const char* ch_ptr = term_seq; *ch_ptr != '\0'; ch_ptr++) {
        symbols.push_back(terminal(*ch_ptr));
    }

    return *this;
}

bool prod_rule::contains(const symbol& target) const {
    return std::any_of(
        begin(), end(), 
        [&](const symbol& symb){ return symb == target; } 
    );
}

size_t prod_rule::prune(const symbol& target) {
    const size_t old_size = size();
    const auto new_end = std::remove(begin(), end(), target);
    symbols.erase(new_end, symbols.end());
    return old_size - size();
}

prod_rule& prod_rule::operator+=(const char term_seq[]) {
    for (const char* ch_ptr = term_seq; *ch_ptr != '\0'; ch_ptr++) {
        symbols.push_back(terminal(*ch_ptr));
    }

    return *this;
}

size_t std::hash<cfg_parser::prod_rule>::
operator()(const cfg_parser::prod_rule& rule) const {
    size_t seed = 0;
    for (const auto& symb : rule) {
        seed ^= std::hash<symbol>()(symb) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}