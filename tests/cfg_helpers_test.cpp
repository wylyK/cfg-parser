#include "cfg.hpp"
#include "cfg_helpers.hpp"

#include <gtest/gtest.h>

using namespace cfg_parser;

class CfgHelpersTest : public testing::Test {

protected:
    static constexpr Cfg::Symbol terminal = 'a';
    Cfg::Symbol nonterminal;
    static constexpr Cfg::Symbol null_nonterminal = nullptr;

    const Cfg::Nonterminal nonterminal_;

    CfgHelpersTest() : nonterminal_(new Cfg()) {
        nonterminal = Cfg::Symbol(nonterminal_);
    }

    ~CfgHelpersTest() {
        delete nonterminal_;
    }
};  

TEST_F(CfgHelpersTest, ChecksTerminals) {
    EXPECT_TRUE(is_terminal(terminal));
    EXPECT_FALSE(is_terminal(nonterminal));
    EXPECT_FALSE(is_terminal(null_nonterminal));
}

TEST_F(CfgHelpersTest, ChecksNonterminals) {
    EXPECT_TRUE(is_nonterminal(nonterminal));
    EXPECT_TRUE(is_nonterminal(null_nonterminal));
    EXPECT_FALSE(is_nonterminal(terminal));
}

TEST_F(CfgHelpersTest, GetsTerminals)  {
    EXPECT_EQ(as_terminal(terminal), 'a');
    EXPECT_ANY_THROW(as_terminal(nonterminal));
    EXPECT_ANY_THROW(as_terminal(null_nonterminal));
}

TEST_F(CfgHelpersTest, GetsNonterminals) {
    EXPECT_EQ(as_nonterminal(nonterminal), nonterminal_);
    EXPECT_EQ(as_nonterminal(null_nonterminal), nullptr);
    EXPECT_ANY_THROW(as_nonterminal(terminal));
}