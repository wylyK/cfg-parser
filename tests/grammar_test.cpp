#include "cfg_parser.hpp"

#include <gtest/gtest.h>
#include <unordered_set>
#include <vector>
#include <utility>

using std::vector;
using std::unordered_set;

using namespace cfg_parser;

class grammar_test : public testing::Test { 

protected:
    static constexpr terminal term = 'a';
    static const  nonterminal nont;
    static const grammar empty_gram;

    static const prod_rule empty_rule;
    static const prod_rule rule1;
    static const prod_rule rule2;
};

const grammar grammar_test::empty_gram;
const nonterminal grammar_test::nont = nonterminal(empty_gram);
const prod_rule grammar_test::empty_rule;
const prod_rule grammar_test::rule1= { term, nont };
const prod_rule grammar_test::rule2= { term, nont, 'b' };

TEST_F(grammar_test, constructs_grammar) {
    ASSERT_TRUE(empty_gram.is_empty());

    grammar gram = { empty_rule, rule1, rule2, rule1 };

    ASSERT_TRUE(gram.size() == 3);
    ASSERT_TRUE(gram.contains(empty_rule));
    ASSERT_TRUE(gram.contains(rule1));
    ASSERT_TRUE(gram.contains(rule2));
    ASSERT_FALSE(gram.contains({ 'x' }));

    grammar other = gram;
    ASSERT_TRUE(other.size() == 3);
    ASSERT_TRUE(other.contains(empty_rule));
    ASSERT_TRUE(other.contains(rule1));
    ASSERT_TRUE(other.contains(rule2));
    ASSERT_FALSE(other.contains({ 'x' }));
}

TEST_F(grammar_test, grammar_cant_have_redundant_rule) {
    grammar gram;
    prod_rule rule = { nonterminal(gram) };
    ASSERT_ANY_THROW((gram = { empty_rule, rule1, { nonterminal(gram) } }));

    gram = {};
    ASSERT_ANY_THROW(gram.insert(rule));

    gram = {};
    ASSERT_ANY_THROW(gram.insert({ nonterminal(gram) }));
}

TEST_F(grammar_test, copied_grammar_own_its_rules) {
    grammar gram1 = { empty_rule, rule1, rule2 };
    unordered_set<const prod_rule*> addresses;
    addresses.insert(&empty_rule);
    addresses.insert(&rule1);
    addresses.insert(&rule2);
    for (const auto& rule : gram1) {
        ASSERT_TRUE(addresses.find(&rule) == addresses.end());
    }

    grammar gram2 = gram1;
    addresses.clear();
    for (const auto& rule : gram1) {
        addresses.insert(&rule);
    }

    for (const auto& rule : gram2) {
        ASSERT_TRUE(addresses.find(&rule) == addresses.end());
    }

    gram2 = {};
    gram2 = gram1;
        for (const auto& rule : gram2) {
        ASSERT_TRUE(addresses.find(&rule) == addresses.end());
    }
}

TEST_F(grammar_test, inserts_and_erases_rule) {
    grammar gram = { empty_rule, rule1 };
    ASSERT_TRUE(gram.insert(rule2));
    ASSERT_EQ(gram.size(), 3);
    ASSERT_TRUE(gram.contains(rule2));

    ASSERT_FALSE(gram.insert(rule2));
    ASSERT_EQ(gram.size(), 3);
    ASSERT_TRUE(gram.contains(rule2));

    ASSERT_FALSE(gram.insert(rule1));
    ASSERT_EQ(gram.size(), 3);
    ASSERT_TRUE(gram.contains(rule1));

    ASSERT_TRUE(gram.erase(rule1));
    ASSERT_EQ(gram.size(), 2);
    ASSERT_FALSE(gram.contains(rule1));

    ASSERT_FALSE(gram.erase(rule1));
    ASSERT_EQ(gram.size(), 2);
    ASSERT_FALSE(gram.contains(rule1));

    ASSERT_TRUE(gram.insert(rule1));
    ASSERT_EQ(gram.size(), 3);
    ASSERT_TRUE(gram.contains(rule1));
}

TEST_F(grammar_test, getters_return_correct_sets) {
    grammar gram = { empty_rule, rule1 };
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>{ term });
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });

    gram.insert(rule2);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>({ term, 'b' }));
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });

    gram.erase(rule2);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>{ term });
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });

    gram.insert({ 'x', nonterminal(gram) });
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>({ term, 'x' }));
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>({ nont, nonterminal(gram) }));

    gram.erase(rule1);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>{ 'x' });
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>({ nonterminal(gram) }));

    gram.insert(rule1);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>({ 'x', term }));
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>({ nonterminal(gram), nont }));

    gram.erase({ 'x', nonterminal(gram) });
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>{ term });
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });

    gram.insert(rule2);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>({ term, 'b' }));
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });

    gram.clear();
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>{});
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{});

    gram.insert(rule1);
    gram.insert(rule2);
    ASSERT_EQ(gram.terminals(),    unordered_set<terminal>({ term, 'b' }));
    ASSERT_EQ(gram.nonterminals(), unordered_set<nonterminal>{ nont });
}