#include "cfg_parser.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <sstream>

using std::string;
using std::vector;
using std::cout;
using std::streambuf;
using std::ostringstream;
using std::unordered_map;
using std::any_of;

using namespace cfg_parser;

class stdout_capturer {

public:
    stdout_capturer() {
        old = cout.rdbuf(buf.rdbuf());
    }

   ~stdout_capturer() {
        cout.rdbuf(old);
    }

    string str() { return buf.str(); }

private:
    streambuf* old;
    ostringstream buf;
};

TEST(parser_test, creates) {
    parser pser;
    ASSERT_ANY_THROW(pser.get_nont("A"));

    pser.create("A");
    const auto A = pser.get_nont("A");
    ASSERT_TRUE(A->is_empty());

    pser.create("B", { "", "hello", A + "A" });
    const auto B = pser.get_nont("B");
    ASSERT_EQ(B->size(), 3);

    ASSERT_TRUE(B->contains(""));
    ASSERT_TRUE(B->contains("hello"));
    ASSERT_TRUE(B->contains(A + "A"));
}

TEST(parser_test, inserts_n_erases_rules) {
    parser pser;
    pser.create("A");
    const auto A = pser.get_nont("A");
    const prod_rule hello = "hello";

    ASSERT_TRUE(pser.insert("A", ""));
    ASSERT_TRUE(pser.insert("A", hello));
    ASSERT_TRUE(pser.insert("A", A + "A"));
    ASSERT_EQ(A->size(), 3);

    ASSERT_FALSE(pser.insert("A", ""));
    ASSERT_FALSE(pser.insert("A", hello));
    ASSERT_FALSE(pser.insert("A", A + "A"));
    ASSERT_EQ(A->size(), 3);

    ASSERT_TRUE(pser.erase("A", "hello"));
    ASSERT_EQ(A->size(), 2);
    ASSERT_FALSE(pser.erase("A", "hello"));
    ASSERT_EQ(A->size(), 2);
}

TEST(parser_test, throws_if_tries_insert_redund_n_foreign_rule) {
    parser pser;
    pser.create("A");
    const auto A =  pser.get_nont("A");
    ASSERT_ANY_THROW(pser.insert("A", { A }));
    const grammar gram;
    ASSERT_ANY_THROW(pser.insert("A", nonterminal(gram) + ""));
}

TEST(parser_test, dyck3) {
    parser pser;
    pser.create("Dyck3", { "" });
    const auto dyck3 = pser.get_nont("Dyck3");
    pser.insert("Dyck3", '(' + dyck3 + ')');
    pser.insert("Dyck3", '[' + dyck3 + ']');
    pser.insert("Dyck3", '{' + dyck3 + '}');
    pser.insert("Dyck3", dyck3 + dyck3);

    stdout_capturer ocapturer;
    pser.print("Dyck3");

    ASSERT_EQ(ocapturer.str(),
        "Dyck3 -> empty rule\n"
        "         ^^^^^^^^^^\n"
        "         ([Dyck3])\n"
        "          ^^^^^^^ \n"
        "         [[Dyck3]]\n"
        "          ^^^^^^^ \n"
        "         {[Dyck3]}\n"
        "          ^^^^^^^ \n"
        "         [Dyck3][Dyck3]\n"
        "         ^^^^^^^^^^^^^^\n"
    );

    ASSERT_TRUE(pser.parse("Dyck3", ""));
    ASSERT_TRUE(pser.parse("Dyck3", "()"));
    ASSERT_TRUE(pser.parse("Dyck3", "[({})]"));
    ASSERT_TRUE(pser.parse("Dyck3", "{[[]]}()(({})){}"));
    ASSERT_TRUE(pser.parse("Dyck3", "(({}))(({}))"));

    ASSERT_FALSE(pser.parse("Dyck3", "("));
    ASSERT_FALSE(pser.parse("Dyck3", "{[[]]}()(({})){}pp"));
    ASSERT_FALSE(pser.parse("Dyck3", "{[[]]}()(({})){})"));
    ASSERT_FALSE(pser.parse("Dyck3", "[{[[]]}()(({})){}"));
    ASSERT_FALSE(pser.parse("Dyck3", "{[[]}()(({})){}"));
}

TEST(parser_test, arithmetic_expression) {
    parser pser;
    pser.create("Term", { "x", "y", "z" });
    const auto term = pser.get_nont("Term");
    pser.insert("Term", term + term);

    pser.create("Sum", { term + " + " + term });
    const auto sum = pser.get_nont("Sum");
    pser.insert("Sum", sum + " + " + term);

    pser.insert("Term", term + '(' + sum + ')');
    pser.insert("Term", '(' + sum + ')' + term);

    pser.create("Expr", { { term }, { sum } });
    const auto expr = pser.get_nont("Expr");

    unordered_map<nonterminal, vector<string>> expected_map;
    expected_map.emplace(term, vector<string>{
        "Term -> x\n"
        "         \n"
        "        y\n"
        "         \n"
        "        z\n"
        "         \n"
        "        [Term][Term]\n"
        "        ^^^^^^^^^^^^\n"
        "        ([Sum])[Term]\n"
        "         ^^^^^ ^^^^^^\n"
        "        [Term]([Sum])\n"
        "        ^^^^^^ ^^^^^ \n"
    });

    expected_map.emplace(sum, vector<string>{
        "Sum -> [Term] + [Term]\n"
        "       ^^^^^^   ^^^^^^\n"
        "       [Sum] + [Term]\n"
        "       ^^^^^   ^^^^^^\n",

        "Sum -> [Sum] + [Term]\n",
        "       ^^^^^   ^^^^^^\n"
        "       [Term] + [Term]\n"
        "       ^^^^^^   ^^^^^^\n"
    });

    expected_map.emplace(expr, vector<string>{
        "Expr -> [Term]\n"
        "        ^^^^^^\n"
        "        [Sum]\n"
        "        ^^^^^\n",

        "Expr -> [Sum]\n"
        "        ^^^^^\n"
        "        [Term]\n"
        "        ^^^^^^\n"
    });

    stdout_capturer ocapturer;
    pser.print("Expr");
    size_t offset = 0;
    expr->dfs(
        [&](nonterminal nont) {
            const auto candidates = expected_map.at(nont);
            ASSERT_LT(offset, ocapturer.str().size());
            const string result = ocapturer.str().substr(offset, candidates[0].size());
            offset += candidates[0].size();
            ASSERT_TRUE(
                any_of(
                    candidates.begin(), candidates.end(),
                    [&](const string& expected) { return expected == result; }
                )
            );
        }
    );

    ASSERT_TRUE(pser.parse("Expr", "x"));
    ASSERT_TRUE(pser.parse("Expr", "xyz"));
    ASSERT_TRUE(pser.parse("Expr", "x + yz"));
    ASSERT_TRUE(pser.parse("Expr", "xz(yz + x)zxy"));
    ASSERT_TRUE(pser.parse("Expr", "(x + xy)zy + x(y + z)"));

    ASSERT_FALSE(pser.parse("Expr", ""));
    ASSERT_FALSE(pser.parse("Expr", "+ xyz"));
    ASSERT_FALSE(pser.parse("Expr", "x + y + dz"));
    ASSERT_FALSE(pser.parse("Expr", "(x + yz) + y"));
    ASSERT_FALSE(pser.parse("Expr", "x(yz)"));
    ASSERT_FALSE(pser.parse("Expr", "((x + yz))xz"));
}