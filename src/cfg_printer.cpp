#include "cfg_printer.hpp"
#include "cfg_helpers.hpp"

#include <iterator>
#include <iostream>

using std::iterator;
using std::string;
using std::cout;
using std::endl;

using namespace cfg_parser;

void CfgPrinter::print_rule(const Cfg::Rule& rule, int padding = 0) {
    string delim_line;

    cout << string(padding, ' ');

    for (const auto& symbol : rule) {
        if (is_terminal(symbol)) {
            cout << as_terminal(symbol);
            delim_line += '*';
            continue;
        }

        string name = name_map[as_nonterminal(symbol)];
        cout << nonterminal_braces.first
             << name
             << nonterminal_braces.second;
        delim_line += string(2 + name.size(), '^');
    }

    cout << endl;
    cout << string(padding, ' ') + delim_line << endl;
}

void CfgPrinter::print_shallow(Cfg::Nonterminal nonterminal) {
    string name = name_map[nonterminal];
    cout << name + " -> ";
    int padding = name.size() + 4;
    auto& rules = nonterminal->rules();

    print_rule(*rules.begin());
    for (auto it = std::next(rules.begin()); it != rules.end(); it++) {
        print_rule(*it, padding);
    }
    
    cout << endl;
}

void CfgPrinter::operator()(Cfg::Nonterminal nonterminal) {
    nonterminal->Dfs([this](Cfg::Nonterminal nt) { print_shallow(nt); } );
}