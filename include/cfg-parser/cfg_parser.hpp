#pragma once

#include "grammar.hpp"

#include <memory>
#include <utility>

namespace cfg_parser {

class parser {

public:
    parser();
   ~parser();

    void create(const std::string& name);
    void create(const std::string& name, std::initializer_list<prod_rule>);
    
    nonterminal get_nont(const std::string& name);

    bool insert(const std::string& name, const prod_rule&);
    bool insert(const std::string& name, prod_rule&&);
    bool erase(const std::string& name, const prod_rule&);

    void print(const std::string& name);
    void print_norm(const std::string& name);

    bool parse(const std::string& name, const std::string& word);
    void parse_file(const std::string& name, const std::string& file_name);

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

} // End of namespace cfg_parser