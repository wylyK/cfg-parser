#include "cfg_parser.hpp"

#include <iostream>

using namespace std;
using namespace cfg_parser;

int main() {
    parser pser;
    pser.create("Dyck3", { "" });

    nonterminal dyck3 = pser.get_nont("Dyck3");

    pser.insert("Dyck3", dyck3 + dyck3);
    pser.insert("Dyck3", '(' + dyck3 + ')');
    pser.insert("Dyck3", '{' + dyck3 + '}');
    pser.insert("Dyck3", '[' + dyck3 + ']');

    pser.print("Dyck3");
    pser.print_norm("Dyck3");

    cout << "Dyck3 "
         << (pser.parse("Dyck3", "") ? "accepts" : "rejects")
         << " empty rule" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "()") ? "accepts" : "rejects")
         << " ()" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "([{}])") ? "accepts" : "rejects")
         << " ([{}])" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "([]){(())}[{([])}]()") ? "accepts" : " rejects")
         << " ([]){(())}[{([])}]()" << '\n';

    cout << "Dyck3 "
         << (pser.parse("Dyck3", "(") ? "accepts" : "rejects")
         << " (" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "]") ? "accepts" : "rejects")
         << " ]" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "[{(([]))}])") ? "accepts" : "rejects")
         << " [{(([]))}])" << '\n';
    cout << "Dyck3 "
         << (pser.parse("Dyck3", "){{}}[(([]))]") ? "accepts" : "rejects")
         << " ){{}}[(([]))]";
}