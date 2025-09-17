#include "cfg_parser.hpp"

#include <gtest/gtest.h>

using namespace cfg_parser;

class symbol_test : public testing::Test {

protected:
    static constexpr terminal term = 'a';
    static const  nonterminal nont;
    static const grammar gram;
    static const symbol term_symb;
    static const symbol nont_symb;
};

const grammar     symbol_test::gram = grammar();
const nonterminal symbol_test::nont = nonterminal(symbol_test::gram);
const symbol symbol_test::term_symb = symbol(term);
const symbol symbol_test::nont_symb = symbol(nont);

TEST_F(symbol_test, terminals_must_be_printable) {
    ASSERT_NO_THROW(terminal('a'));
    ASSERT_ANY_THROW(terminal(0));
}

TEST_F(symbol_test, terminal_relations_work) {
    ASSERT_EQ(term, 'a');
    ASSERT_NE(term, 'b');
    ASSERT_LE(term, 'b');
    ASSERT_GE(term, ' ');
}

TEST_F(symbol_test, symbols_hold_correct_alt) {
    ASSERT_TRUE(term_symb.is_term());
    ASSERT_FALSE(term_symb.is_nont());
    ASSERT_TRUE(nont_symb.is_nont());
    ASSERT_FALSE(nont_symb.is_term());
    
    ASSERT_EQ(term_symb, symbol(term));
    ASSERT_EQ(nont_symb, nonterminal(nont));
}

TEST_F(symbol_test, symbols_get_alt) {
    ASSERT_EQ(symbol(term).as_term(), 'a');
    ASSERT_NE(symbol('b').as_term(), term);
    ASSERT_EQ(symbol(nont).as_nont(), nont);
    ASSERT_ANY_THROW(symbol(term).as_nont());
    ASSERT_ANY_THROW(symbol(nont).as_term());
}
