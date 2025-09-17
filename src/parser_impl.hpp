#pragma once

#include "cfg_parser.hpp"

#include <utility>
#include <unordered_map>
#include <initializer_list>

namespace cfg_parser {

class parser::impl {

public:    
    struct gram_family;
    class normalizer;

    std::unordered_map<std::string, gram_family> gram_map;
    std::unordered_map<nonterminal, std::string> name_map;
    std::unordered_map<nonterminal, std::string> name_map_for_norm;
    std::unordered_map<std::string, const grammar> singleton_map;
    
    bool is_foreign(const nonterminal) const;
    void throw_if_has_foreign(const prod_rule&) const;

    grammar& get_if_exists(const std::string& name);
    const grammar& get_norm_if_exists(const std::string& name);

    // Prints rules of the nont
    void print_shallow(nonterminal);

    // The nont is reachable from a normalized gram
    void print_shallow_for_norm(nonterminal);

private:
    // __Helpers for print_shallow and print_shallow_for_norm__

    static bool greater_than(const symbol&, const symbol&);
    struct rule_comparator;

    void print(const prod_rule&, size_t padding);
    void print(const prod_rule&, size_t padding, size_t delim_padding);
    void print_for_norm(const prod_rule&, size_t padding);
    void print_for_norm(const prod_rule&, size_t padding, size_t delim_padding);
};

struct parser::impl::gram_family {
    grammar gram;
    grammar norm_form;
    bool    norm_form_valid = false;
    std::vector<std::unique_ptr<grammar>> owned_grams; // excluding norm_form itself
    
    impl* pimpl;

    gram_family(impl* pimpl) : pimpl(pimpl) {}
    gram_family(const gram_family&) = delete;
    gram_family(gram_family&&)      = delete;

    const grammar& normalized_form();
};

}