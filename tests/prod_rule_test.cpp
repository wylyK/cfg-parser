#include "cfg_parser.hpp"

#include <gtest/gtest.h>
#include <string>

using std::string;

using namespace cfg_parser;

class prod_rule_test : public testing::Test {

protected:
    static constexpr terminal term1 = 'a';
    static constexpr terminal term2 = '1';
    static const  nonterminal nont1;
    static const  nonterminal nont2;
    static const grammar gram1;
    static const grammar gram2;
    static const symbol term_symb;
    static const symbol nont_symb;
};

const grammar prod_rule_test::gram1 = grammar();
const grammar prod_rule_test::gram2 = grammar();
const nonterminal prod_rule_test::nont1(gram1);
const nonterminal prod_rule_test::nont2(gram2);
const symbol prod_rule_test::term_symb('a');
const symbol prod_rule_test::nont_symb(prod_rule_test::nont1);

TEST_F(prod_rule_test, constructs_empty_rule) {
    const prod_rule empty1;
    const prod_rule empty2 = string();
    const prod_rule empty3 = "";
    const prod_rule empty4 = {};
    const prod_rule empty5(0, 'a');
    const prod_rule empty6 = empty5;

    ASSERT_TRUE(empty1.is_empty());
    ASSERT_TRUE(empty2.is_empty());
    ASSERT_TRUE(empty3.is_empty());
    ASSERT_TRUE(empty4.is_empty());
    ASSERT_TRUE(empty5.is_empty());
    ASSERT_TRUE(empty6.is_empty());
}

TEST_F(prod_rule_test, constructs_term_seq) {
    const prod_rule term_seq1 = "hello";
    const prod_rule term_seq2 = string("hello");
    const prod_rule term_seq3 = {'h', 'e', 'l', 'l', 'o'};
    const prod_rule term_seq4 = term_seq3;

    ASSERT_FALSE(term_seq1.is_empty());
    ASSERT_FALSE(term_seq2.is_empty());
    ASSERT_FALSE(term_seq3.is_empty());
    ASSERT_FALSE(term_seq4.is_empty());
}

TEST_F(prod_rule_test, ctor_fills) {
    const prod_rule rule1(1, 'a');
    const prod_rule rule2(2, term1);
    const prod_rule rule3(3, nont2);
    const prod_rule rule4(4, term_symb);

    ASSERT_EQ(rule1.size(), 1);
    ASSERT_EQ(rule2.size(), 2);
    ASSERT_EQ(rule3.size(), 3);
    ASSERT_EQ(rule4.size(), 4);

    for (const auto& symb : rule1) {
        ASSERT_EQ(symb, 'a');
    }

    for (const auto& symb : rule2) {
        ASSERT_EQ(symb, term1);
    }

    for (const auto& symb : rule3) {
        ASSERT_EQ(symb, nont2);
    }

    for (const auto& symb : rule4) {
        ASSERT_EQ(symb, term_symb);
    }
}

TEST_F(prod_rule_test, constructs_complex_rule) {
    const prod_rule rule1 = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    const prod_rule rule2 = rule1;

    ASSERT_EQ(rule1.size(), 7);
    ASSERT_EQ(rule2.size(), 7);
    ASSERT_FALSE(rule1.is_empty());
    ASSERT_FALSE(rule2.is_empty());
    ASSERT_ANY_THROW(rule1.at(7));
    ASSERT_ANY_THROW(rule2.at(7));

    ASSERT_EQ(rule1.at(0), 'a');
    ASSERT_EQ(rule1.at(1), term1);
    ASSERT_EQ(rule1.at(2), nont1);
    ASSERT_EQ(rule1.at(3), term2);
    ASSERT_EQ(rule1.at(4), nont2);
    ASSERT_EQ(rule1.at(5), term_symb);
    ASSERT_EQ(rule1.at(6), nont_symb);

    ASSERT_EQ(rule2.at(0), 'a');
    ASSERT_EQ(rule2.at(1), term1);
    ASSERT_EQ(rule2.at(2), nont1);
    ASSERT_EQ(rule2.at(3), term2);
    ASSERT_EQ(rule2.at(4), nont2);
    ASSERT_EQ(rule2.at(5), term_symb);
    ASSERT_EQ(rule2.at(6), nont_symb);
}

TEST_F(prod_rule_test, ctor_deep_copies) {
    const prod_rule rule  = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    const prod_rule other = rule;

    ASSERT_NE(&rule.at(0), &other.at(0));
    ASSERT_NE(&rule.at(1), &other.at(1));
    ASSERT_NE(&rule.at(2), &other.at(2));
    ASSERT_NE(&rule.at(3), &other.at(3));
    ASSERT_NE(&rule.at(4), &other.at(4));
    ASSERT_NE(&rule.at(5), &other.at(5));
    ASSERT_NE(&rule.at(6), &other.at(6));
}

TEST_F(prod_rule_test, ctor_transfers_ownership) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };

    std::vector<const symbol*> addresses;
    for (const auto& symb : rule) {
        addresses.push_back(&symb);
    }
    ASSERT_EQ(addresses.size(), 7);

    const prod_rule other = std::move(rule);

    ASSERT_EQ(other.size(), 7);
    for (size_t i = 0; i < other.size(); i++) {
        ASSERT_EQ(&other.at(i), addresses[i]);
    }
}

TEST_F(prod_rule_test, assigns_to_empty_rule) {
    prod_rule rule = { 'a' };
    rule = prod_rule();
    ASSERT_TRUE(rule.is_empty());

    rule = {};
    ASSERT_TRUE(rule.is_empty());

    rule = "";
    ASSERT_TRUE(rule.is_empty());

    rule = string();
    ASSERT_TRUE(rule.is_empty());
}

TEST_F(prod_rule_test, assigns_to_single_symb) {
    prod_rule rule;
    rule = 'a';
    ASSERT_EQ(rule.size(), 1);
    ASSERT_EQ(rule.at(0), 'a');

    rule = term1;
    ASSERT_EQ(rule.size(), 1);
    ASSERT_EQ(rule.at(0), term1);

    rule = nont1;
    ASSERT_EQ(rule.size(), 1);
    ASSERT_EQ(rule.at(0), nont1);

    rule = { nont2 };
    ASSERT_EQ(rule.size(), 1);
    ASSERT_EQ(rule.at(0), nont2);
}

TEST_F(prod_rule_test, assigns_to_term_seq) {
    string hello = "hello";
    prod_rule term_seq;
    term_seq = hello;
    ASSERT_EQ(term_seq.size(), hello.size());
    for (size_t i = 0; i < hello.size(); i++) {
        ASSERT_EQ(term_seq.at(i), hello[i]);
    }

    term_seq = "hello";
    for (size_t i = 0; i < hello.size(); i++) {
        ASSERT_EQ(term_seq.at(i), hello[i]);
    }
}

TEST_F(prod_rule_test, assignment_fills) {
    prod_rule rule;
    rule.assign(1, 'a');
    ASSERT_EQ(rule.size(), 1);
    for (const auto& symb : rule) {
        ASSERT_EQ(symb, 'a');
    }

    rule.assign(2, term1);
    ASSERT_EQ(rule.size(), 2);
    for (const auto& symb : rule) {
        ASSERT_EQ(symb, term1);
    }

    rule.assign(3, nont2);
    ASSERT_EQ(rule.size(), 3);
    for (const auto& symb : rule) {
        ASSERT_EQ(symb, nont2);
    }

    rule.assign(4, term_symb);
    ASSERT_EQ(rule.size(), 4);
    for (const auto& symb : rule) {
        ASSERT_EQ(symb, term_symb);
    }
}

TEST_F(prod_rule_test, assigns_to_complex_rule) {
    prod_rule rule;
    rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };

    ASSERT_EQ(rule.at(0), 'a');
    ASSERT_EQ(rule.at(1), term1);
    ASSERT_EQ(rule.at(2), nont1);
    ASSERT_EQ(rule.at(3), term2);
    ASSERT_EQ(rule.at(4), nont2);
    ASSERT_EQ(rule.at(5), term_symb);
    ASSERT_EQ(rule.at(6), nont_symb);
}

TEST_F(prod_rule_test, assignment_deep_copies) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    prod_rule other;
    other = rule;

    ASSERT_NE(&rule.at(0), &other.at(0));
    ASSERT_NE(&rule.at(1), &other.at(1));
    ASSERT_NE(&rule.at(2), &other.at(2));
    ASSERT_NE(&rule.at(3), &other.at(3));
    ASSERT_NE(&rule.at(4), &other.at(4));
    ASSERT_NE(&rule.at(5), &other.at(5));
    ASSERT_NE(&rule.at(6), &other.at(6));
}

TEST_F(prod_rule_test, assignment_transfers_ownership) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };

    std::vector<const symbol*> addresses;
    for (const auto& symb : rule) {
        addresses.push_back(&symb);
    }
    ASSERT_EQ(addresses.size(), 7);

    prod_rule other;
    other = std::move(rule);

    ASSERT_EQ(other.size(), 7);
    for (size_t i = 0; i < other.size(); i++) {
        ASSERT_EQ(&other.at(i), addresses[i]);
    }
}

TEST_F(prod_rule_test, concat_assigning_empty_rule_has_no_effect) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    rule += prod_rule();
    rule += {};
    rule += "";
    rule += string();

    ASSERT_EQ(rule.size(), 7);
    ASSERT_EQ(rule.at(0), 'a');
    ASSERT_EQ(rule.at(1), term1);
    ASSERT_EQ(rule.at(2), nont1);
    ASSERT_EQ(rule.at(3), term2);
    ASSERT_EQ(rule.at(4), nont2);
    ASSERT_EQ(rule.at(5), term_symb);
    ASSERT_EQ(rule.at(6), nont_symb);
}

TEST_F(prod_rule_test, concat_assigns_single_symb) {
    prod_rule rule = { 'a' };
    rule += term1;
    ASSERT_EQ(rule.size(), 2);
    ASSERT_EQ(rule.at(0), 'a');
    ASSERT_EQ(rule.at(1), term1);

    rule += nont1;
    ASSERT_EQ(rule.size(), 3);
    ASSERT_EQ(rule.back(), nont1);

    rule += { term2 };
    ASSERT_EQ(rule.size(), 4);
    ASSERT_EQ(rule.back(), term2);

    rule += { nont2 };
    ASSERT_EQ(rule.size(), 5);
    ASSERT_EQ(rule.back(), nont2);

    rule += term_symb;
    ASSERT_EQ(rule.size(), 6);
    ASSERT_EQ(rule.back(), term_symb);

    rule += { nont_symb };
    ASSERT_EQ(rule.size(), 7);
    ASSERT_EQ(rule.back(), nont_symb);
}

TEST_F(prod_rule_test, concat_assigns_term_seq) {
    prod_rule rule = "hello ";
    rule += "world";

    string term_seq("hello world");
    ASSERT_EQ(rule.size(), term_seq.size());
    for (size_t i = 0; i < rule.size(); i++) {
        ASSERT_EQ(rule.at(i), term_seq.at(i));
    }
}

TEST_F(prod_rule_test, concat_assigns_complex_rule) {
    prod_rule rule = { 'a', term1, nont1 };
    rule += { term2, nont2, term_symb, nont_symb };

    ASSERT_EQ(rule.size(), 7);
    ASSERT_EQ(rule.at(0), 'a');
    ASSERT_EQ(rule.at(1), term1);
    ASSERT_EQ(rule.at(2), nont1);
    ASSERT_EQ(rule.at(3), term2);
    ASSERT_EQ(rule.at(4), nont2);
    ASSERT_EQ(rule.at(5), term_symb);
    ASSERT_EQ(rule.at(6), nont_symb);
}

TEST_F(prod_rule_test, concat_empty_rules_yields_empty_rule) {
    prod_rule rule = prod_rule() + "";
    ASSERT_TRUE(rule.is_empty());

    rule = string() + prod_rule();
    ASSERT_TRUE(rule.is_empty());
}

TEST_F(prod_rule_test, concats_term_seqs) {
    prod_rule rule  = "hello ";
    prod_rule other = rule + string("world");

    string term_seq("hello world");
    ASSERT_EQ(other.size(), term_seq.size());
    for (size_t i = 0; i < other.size(); i++) {
        ASSERT_EQ(other.at(i), term_seq.at(i));
    }
}

TEST_F(prod_rule_test, concats_complex_rules) {
    prod_rule rule1 = { 'a', term1, nont1 };
    prod_rule rule2 = { term2, nont2, term_symb, nont_symb };
    prod_rule rule3 = rule1 + rule2;

    ASSERT_EQ(rule3.size(), 7);
    ASSERT_EQ(rule3.at(0), 'a');
    ASSERT_EQ(rule3.at(1), term1);
    ASSERT_EQ(rule3.at(2), nont1);
    ASSERT_EQ(rule3.at(3), term2);
    ASSERT_EQ(rule3.at(4), nont2);
    ASSERT_EQ(rule3.at(5), term_symb);
    ASSERT_EQ(rule3.at(6), nont_symb);
}

TEST_F(prod_rule_test, prunes) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    ASSERT_EQ(rule.prune(term2), 1);
    ASSERT_EQ(rule.size(), 6);
    ASSERT_EQ(rule.at(0), 'a');
    ASSERT_EQ(rule.at(1), term1);
    ASSERT_EQ(rule.at(2), nont1);
    ASSERT_EQ(rule.at(3), nont2);
    ASSERT_EQ(rule.at(4), term_symb);
    ASSERT_EQ(rule.at(5), nont_symb);

    ASSERT_EQ(rule.prune('a'), 3);
    ASSERT_EQ(rule.size(), 3);
    ASSERT_EQ(rule.at(0), nont1);
    ASSERT_EQ(rule.at(1), nont2);
    ASSERT_EQ(rule.at(2), nont_symb);

    ASSERT_EQ(rule.prune('x'), 0);
    ASSERT_EQ(rule.size(), 3);
    ASSERT_EQ(rule.at(0), nont1);
    ASSERT_EQ(rule.at(1), nont2);
    ASSERT_EQ(rule.at(2), nont_symb);

    ASSERT_EQ(rule.prune(nont2), 1);
    ASSERT_EQ(rule.size(), 2);
    ASSERT_EQ(rule.at(0), nont1);
    ASSERT_EQ(rule.at(1), nont_symb);

    ASSERT_EQ(rule.prune(nont_symb), 2);
    ASSERT_TRUE(rule.is_empty());

    ASSERT_EQ(rule.prune('x'), 0);
    ASSERT_TRUE(rule.is_empty());
}

TEST_F(prod_rule_test, prunes_given_cond) {
    prod_rule rule = { 'a', term1, nont1, term2, nont2, term_symb, nont_symb };
    ASSERT_EQ(rule.prune_if([](const auto& symb) { return symb == 'a'; }), 3);
    ASSERT_EQ(rule.size(), 4);
    ASSERT_EQ(rule.at(0), nont1);
    ASSERT_EQ(rule.at(1), term2);
    ASSERT_EQ(rule.at(2), nont2);
    ASSERT_EQ(rule.at(3), nont_symb);
}
