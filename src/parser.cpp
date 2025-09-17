#include "parser_impl.hpp"
#include "parser_impl_normalizer.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>

using std::unordered_set;
using std::unordered_map;
using std::string;
using std::pair;
using std::invalid_argument;

using namespace cfg_parser;

parser:: parser() : pimpl(std::make_unique<impl>()) {}
parser::~parser() = default;

// Creates a new empty grammar

void parser::create(const string& name) {
    if (name.empty())
        throw invalid_argument("Name can't be empty.");

    const auto [it, inserted] = pimpl->gram_map.emplace(
        name, pimpl.get()
    );

    if (!inserted)
        throw invalid_argument(name + " already exists.");

    pimpl->name_map.emplace(nonterminal(it->second.gram), name);
    pimpl->name_map_for_norm.emplace(nonterminal(it->second.norm_form), name);
}

// Creates a new grammar initialized with init
void parser::create(
    const string& name, std::initializer_list<prod_rule> init
) {
    if (name.empty())
        throw invalid_argument("Name can't be empty.");

    const auto [it, inserted] = pimpl->gram_map.emplace(
        name, pimpl.get()
    );

    if (!inserted) 
        throw invalid_argument(name + " already exists.");

    for (const auto& rule : init) {
        pimpl->throw_if_has_foreign(rule);
        it->second.gram.insert(rule);
    }

    pimpl->name_map.emplace(nonterminal(it->second.gram), name);
    pimpl->name_map_for_norm.emplace(nonterminal(it->second.norm_form), name);
}

nonterminal parser::get_nont(const string& name) {
    return nonterminal(pimpl->get_if_exists(name));
 }

bool parser::insert(const string& name, const prod_rule& rule) {
    auto& gram = pimpl->get_if_exists(name);
    pimpl->throw_if_has_foreign(rule);
    return gram.insert(rule);
}

bool parser::insert(const string& name, prod_rule&& rule) {
    auto& gram = pimpl->get_if_exists(name);
    pimpl->throw_if_has_foreign(rule);    
    return gram.insert(std::move(rule));
}

bool parser::erase(const string& name, const prod_rule& rule) {
    auto& gram = pimpl->get_if_exists(name);
    return gram.erase(rule);
}

void parser::print(const string& name) {
    const auto& gram = pimpl->get_if_exists(name);
    gram.dfs(
        [this](nonterminal nont) { 
            pimpl->print_shallow(nont); 
        }
    );
}

void parser::print_norm(const string& name) {
    const auto& gram = pimpl->get_norm_if_exists(name);
    gram.dfs(
        [this](nonterminal nont) { 
            pimpl->print_shallow_for_norm(nont);
        }
    );
}

namespace internal_parser {

// norm_form must be reachable from a normalized grammar

struct derives {
    using input_type = pair<nonterminal, string>;
    struct input_hash { size_t operator()(const input_type&) const; };
    unordered_set<input_type, input_hash> memo;
    bool operator()(const grammar&, const string&);
};

size_t derives::input_hash::operator()(const input_type& input) const {
    const size_t seed  = std::hash<nonterminal>()(input.first);
    const size_t value = std::hash<string>()(input.second);
    return seed ^ value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

bool derives::operator()(const grammar& norm_form, const string& text) {
    if (text.empty()) return norm_form.contains("");
    if (memo.count({ nonterminal(norm_form), text })) {
        return true;
    }

    if (text.size() == 1) {
        for (const auto& term : norm_form.terminals()) {
            if (text.front() == term) {
                memo.emplace(nonterminal(norm_form), text);
                return true;
            }
        }

        return false;
    }

    for (const auto& rule : norm_form) {
        if (rule.size() != 2) continue;
        const auto& front_gram = *rule.front().as_nont();
        const auto&  back_gram =  *rule.back().as_nont();
        for (size_t left_len = 1; left_len < text.size(); left_len++) {
            if (operator()(front_gram, text.substr(0, left_len)) &&
                operator()(back_gram,  text.substr(left_len))) {
                memo.emplace(nonterminal(norm_form), text);
                return true;
            }
        }
    }

    return false;
}

} // End of internal_parser

bool parser::parse(const string& name, const string& text) {
    const auto& norm_form = pimpl->get_norm_if_exists(name);
    return internal_parser::derives()(norm_form, text);
}

void parser::parse_file(const string& name, const string& file_name) {
    std::ifstream text_list("file_name");

    if (!text_list) {
        throw invalid_argument("Could'nt open file.");
    }

    string text;
    while (std::getline(text_list, text)) {
        std::cout << name
                  << (parse(name, text) ? " accepts " : " rejects ")
                  << text << '\n';
    }
}