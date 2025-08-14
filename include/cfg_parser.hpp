#pragma once

#include "cfg.hpp"
#include "cfg_rule.hpp"

#include <set>
#include <map>
#include <string>
#include <memory>

namespace cfg_parser {

class CfgParser {

public:
    void create(const std::string& name, std::initializer_list<Cfg::Rule> rules);
    
    Cfg::Nonterminal get(const std::string& name);

    bool  insert(const std::string& name, const Cfg::Rule& rule);
    size_t erase(const std::string& name, const Cfg::Rule& rule);

    void print(const std::string& name);
    void print_norm(const std::string& name);

    bool parse(const std::string& name, const std::string& word);
    void parse_file(const std::string& name, const std::string& file_name);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

}