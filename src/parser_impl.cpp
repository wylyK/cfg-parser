#include "parser_impl.hpp"
#include "parser_impl_normalizer.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

using std::unordered_set;
using std::unordered_map;
using std::string;
using std::pair;
using std::invalid_argument;

using namespace cfg_parser;

bool parser::impl::
is_foreign(const nonterminal nont) const {
    return !name_map.count(nont);
}

void parser::impl::
throw_if_has_foreign(const prod_rule& rule) const {
    if (std::any_of(
        rule.begin(), rule.end(),
        [this](const symbol& symb) {
            return symb.is_nont() &&
                   is_foreign(symb.as_nont());
        }
    )) throw invalid_argument("Grammar can't have foreign nonterminals.");
}

grammar& parser::impl::
get_if_exists(const string& name) {
    const auto it = gram_map.find(name);
    if (it == gram_map.end()) 
        throw invalid_argument(name + " doesn't exist.");

    return it->second.gram;
}

const grammar& parser::impl::
get_norm_if_exists(const string& name) {
    const auto it = gram_map.find(name);
    if (it == gram_map.end())
        throw invalid_argument(name + " doesn't exist.");

    return it->second.normalized_form();
}

const grammar& parser::impl::gram_family::normalized_form() {
    if (!norm_form_valid) {
        norm_form.clear();
        normalizer nzer(*this);
        nzer.normalize();
        norm_form_valid = true;
    }

    return norm_form;
}

bool parser::impl::greater_than(const symbol& lhs, const symbol& rhs) {
    if (lhs.is_term()) {
        if (rhs.is_nont())
            return false; // Terminals are never greater than nonterminals

        return lhs.as_term() > rhs.as_term();
    }

    if (rhs.is_term()) return true; // Nonterminals are always greater than terminals
    
    /* Although this ordering on nonterminals seems arbitrary,
    it ensures rule comparision satisfies
    !(rule_lhs < rule_rhs) && !(rule_rhs < rule_lhs) <=> rule_lhs == rule_rhs,
    where < is rule_comparator::operator(). */ 
    return &*lhs.as_nont() > &*rhs.as_nont();
}

struct parser::impl::rule_comparator {
    bool operator()(const prod_rule& lhs, const prod_rule& rhs) {
        size_t num_lhs_nonts = 0;
        size_t num_rhs_nonts = 0;
        
        for (const auto& symb : lhs) {
            if (symb.is_nont()) num_lhs_nonts++;
        }

        for (const auto& symb : rhs) {
            if (symb.is_nont()) num_rhs_nonts++;
        }

        if (num_lhs_nonts != num_rhs_nonts) return num_lhs_nonts > num_rhs_nonts;
        if (lhs.size() != rhs.size()) return lhs.size() > rhs.size();

        for (size_t i = 0; i < lhs.size(); i++) {
            if (lhs[i] == rhs[i]) continue;
            if (greater_than(lhs[i], rhs[i])) return true;
            else return false;
        }

        return false;
    }
};

void parser::impl::print(const prod_rule& rule, size_t padding) {
    string delim_line;
    cout << string(padding, ' ');

    for (const auto& symb : rule) {
        if (symb.is_term()) {
            cout << symb.as_term();
            delim_line += ' ';
            continue;
        }

        string name = name_map[symb.as_nont()];
        cout << '[' << name << ']';
        delim_line += string(2 + name.size(), '^');
    }

    if (rule.is_empty()) {
        cout << "empty rule";
        delim_line = string(10, '^');
    }

    cout << '\n'
         << string(padding, ' ')
         << delim_line
         << '\n';
}

void parser::impl::print(
    const prod_rule& rule,
    size_t padding,
    size_t delim_padding
) {
    string delim_line;
    cout << string(padding, ' ');

    for (const auto& symb : rule) {
        if (symb.is_term()) {
            cout << symb.as_term();
            delim_line += ' ';
            continue;
        }

        string name = name_map[symb.as_nont()];
        cout << '[' << name << ']';
        delim_line += string(2 + name.size(), '^');
    }

    if (rule.is_empty()) {
        cout << "empty rule";
        delim_line = string(10, '^');
    }

    cout << '\n'
         << string(delim_padding, ' ')
         << delim_line
         << '\n';
}

void parser::impl::print_for_norm(const prod_rule& rule, size_t padding) {
    string delim_line;
    cout << string(padding, ' ');

    for (const auto& symb : rule) {
        if (symb.is_term()) {
            cout << symb.as_term();
            delim_line += ' ';
            continue;
        }

        string name = name_map_for_norm[symb.as_nont()];
        cout << '(' << name << ')';
        delim_line += string(2 + name.size(), '^');
    }

    if (rule.is_empty()) {
        cout << "empty rule";
        delim_line = string(10, '^');
    }

    cout << '\n'
         << string(padding, ' ')
         << delim_line
         << '\n';
}

void parser::impl::print_for_norm(
    const prod_rule& rule,
    size_t padding,
    size_t delim_padding
) {
    string delim_line;
    cout << string(padding, ' ');

    for (const auto& symb : rule) {
        if (symb.is_term()) {
            cout << symb.as_term();
            delim_line += ' ';
            continue;
        }

        string name = name_map_for_norm[symb.as_nont()];
        cout << '(' << name << ')';
        delim_line += string(2 + name.size(), '^');
    }

    if (rule.is_empty()) {
        cout << "empty rule";
        delim_line = string(10, '^');
    }

    cout << '\n'
         << string(delim_padding, ' ')
         << delim_line
         << '\n';
}

void parser::impl::print_shallow(nonterminal nont) {
    string  name = name_map[nont];
    cout << name;
    if (nont->is_empty()) {
        cout << " is empty.";
        return;
    }

    cout << " -> ";
    size_t padding = name.size() + 4;

    std::priority_queue<
        prod_rule,
        std::vector<prod_rule>,
        rule_comparator
    > q(nont->begin(), nont->end());

    print(q.top(), 0, padding);
    q.pop();
    while(!q.empty()) {
        print(q.top(), padding);
        q.pop();
    }
}

void parser::impl::print_shallow_for_norm(nonterminal nont) {
    string  name = name_map_for_norm[nont];
    cout << name;

    if (nont->is_empty()) {
        cout << " is empty.";
        return;
    }

    cout << " -> ";
    size_t padding = name.size() + 4;

    std::priority_queue<
        prod_rule,
        std::vector<prod_rule>,
        rule_comparator
    > q(nont->begin(), nont->end());

    print_for_norm(q.top(), 0, padding);
    q.pop();
    while(!q.empty()) {
        print_for_norm(q.top(), padding);
        q.pop();
    }
}