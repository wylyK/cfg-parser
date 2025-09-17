#include "cfg_parser.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::unique_ptr;

using namespace cfg_parser;

class grammar_traverser_test : public testing::Test {
    // Helper for assert_isomorphoric
    prod_rule get_copy(prod_rule, const unordered_map<nonterminal, unique_ptr<grammar>>& mut_copies);

protected:
    static vector<grammar> nodes;

    static void SetUpTestSuite() {
        nodes.assign(22, grammar());

        nodes[0].insert({
            nonterminal(nodes[1]), // tree root
            nonterminal(nodes[13]),
            nonterminal(nodes[14])
        });

        nodes[1].insert({  nonterminal(nodes[20]) });
        nodes[20].insert({ nonterminal(nodes[19]), nonterminal(nodes[2]) });
        nodes[19].insert({ nonterminal(nodes[3]),  nonterminal(nodes[4]) });
        nodes[3].insert({  nonterminal(nodes[5]) });
        nodes[5].insert({  nonterminal(nodes[4]) });
        nodes[2].insert({  nonterminal(nodes[6]) });
        nodes[6].insert({  nonterminal(nodes[7]),  nonterminal(nodes[8]) });
        nodes[7].insert({  nonterminal(nodes[9]) });
        nodes[9].insert({  nonterminal(nodes[10]) });
        nodes[8].insert({  nonterminal(nodes[11]) });
        nodes[11].insert({ nonterminal(nodes[12]) });

        nodes[13].insert({ nonterminal(nodes[14]) });
        nodes[14].insert({ nonterminal(nodes[15]) });
        nodes[15].insert({ nonterminal(nodes[16]), nonterminal(nodes[17]) });
        nodes[16].insert({ nonterminal(nodes[16]), nonterminal(nodes[21]) });
        nodes[17].insert({ nonterminal(nodes[18]) });
        nodes[18].insert({ nonterminal(nodes[15]) });
    }   

    class dfs_output_tester {
        unordered_set<nonterminal> visited;

        bool has_succ_not_yet_visited(nonterminal);

        /* Tests whether the output interval [begin, output.end()] is a valid
        dfs visitation history, where output.begin() < begin <= output.end() */
        void assert_valid_output_recur(
            vector<nonterminal>::const_iterator& begin,
            const vector<nonterminal>& output
        );

    public:
        // Tests whether output is a valid start.dfs visitation history
        void assert_valid_output(const grammar& start, const vector<nonterminal>& output);

        // Tests whether output_on_input_tree is a valid start.dfs_bottom_up visitation history
        void assert_valid_bottom_up_output(const grammar& start, const vector<nonterminal>& output_on_input_tree);
    };

    void assert_isomorphism(
        const grammar& gram, const grammar& copy,
        const unordered_map<nonterminal, unique_ptr<grammar>>& mut_copies
    );
};

vector<grammar> grammar_traverser_test::nodes;

bool grammar_traverser_test::dfs_output_tester::
has_succ_not_yet_visited(nonterminal nont) {
    const unordered_set<nonterminal> successors = nont->nonterminals();
    return std::find_if(
        successors.begin(), successors.end(),
        [&](nonterminal next) { return !visited.count(next); }
    ) != successors.end();
}

void grammar_traverser_test::dfs_output_tester::
assert_valid_output_recur(
    vector<nonterminal>::const_iterator& begin,
    const vector<nonterminal>& output
) {
    const auto prev = *(begin - 1);
    const unordered_set<nonterminal> successors = prev->nonterminals();
    while(has_succ_not_yet_visited(prev)) {
        ASSERT_NE(begin, output.end());

        // *begin must a successor not yet visited
        ASSERT_TRUE(successors.count(*begin));
        ASSERT_TRUE(visited.insert(*begin).second);

        begin++;
        assert_valid_output_recur(begin, output);
    }
}

void grammar_traverser_test::dfs_output_tester::
assert_valid_output(const grammar& start, const vector<nonterminal>& output) {
    ASSERT_EQ(output.front(), nonterminal(start));
    visited.insert(*output.begin());
    vector<nonterminal>::const_iterator begin = output.begin() + 1;
    assert_valid_output_recur(begin, output);
    visited.clear();
}

void grammar_traverser_test::dfs_output_tester::
assert_valid_bottom_up_output(const grammar& start, const vector<nonterminal>& output /* on input tree */ ) {
    ASSERT_TRUE((*output.begin())->nonterminals().empty());
    visited.insert(*output.begin());

    unordered_set<nonterminal> succs_to_match;

    auto prev = output.begin();
    for (auto curr = output.begin() + 1; curr != output.end(); curr++) {
        ASSERT_TRUE(visited.insert(*curr).second);
        ASSERT_FALSE(has_succ_not_yet_visited(*curr));
    
        if ((*curr)->nonterminals().count(*prev)) {
            succs_to_match.insert(*prev);
            if ((*curr)->nonterminals() == succs_to_match) {
                succs_to_match.clear();
            }

            succs_to_match.erase(*prev);
        } else {
            succs_to_match.insert(*prev);
        }

        prev = curr;
    }

    ASSERT_TRUE(succs_to_match.empty());
    ASSERT_EQ(output.back(), nonterminal(nodes[1]));
    
    size_t count = 0;
    start.dfs([&](nonterminal nont) { count++; });
    ASSERT_EQ(output.size(), count);

    visited.clear();
}

prod_rule grammar_traverser_test::get_copy(
    prod_rule rule,
    const unordered_map<nonterminal, unique_ptr<grammar>>& mut_copies
) {
    for (auto& symb : rule) {
        if (symb.is_term()) continue;
        symb = nonterminal(*mut_copies.at(symb.as_nont()).get());
    }

    return rule;
}

void grammar_traverser_test::assert_isomorphism(
    const grammar& gram,
    const grammar& copy,
    const unordered_map<nonterminal, unique_ptr<grammar>>& mut_copies
) {
    ASSERT_EQ(gram.size(), copy.size());
    for (const auto& rule : gram) {
        ASSERT_TRUE(copy.contains(get_copy(rule, mut_copies)));
    }
}

TEST_F(grammar_traverser_test, dfs_is_depth_first) {
    vector<nonterminal> output;
    nodes[0].dfs([&](nonterminal nont) { output.push_back(nont); });
    dfs_output_tester().assert_valid_output(nodes[0], output);
}

TEST_F(grammar_traverser_test, dfs_bottom_up_is_bottom_up) {
    vector<nonterminal> output;
    nodes[1].dfs_bottom_up([&](nonterminal nont) { output.push_back(nont); });
    dfs_output_tester().assert_valid_bottom_up_output(nodes[1], output);
}

TEST_F(grammar_traverser_test, nodes_are_reachable_from_themselves) {
    for (const auto& node : nodes) {
        ASSERT_TRUE(node.reachable_from(nonterminal(node)));
    }
}

TEST_F(grammar_traverser_test, nodes_are_reachable_from_start) {
    for (const auto& node : nodes) {
        ASSERT_TRUE(node.reachable_from(nonterminal(nodes[0])));
    }
}

TEST_F(grammar_traverser_test, new_gram_unreachable_from_any) {
    grammar gram;
    for (const auto& node : nodes) {
        ASSERT_FALSE(gram.reachable_from(nonterminal(node)));
    }
}

TEST_F(grammar_traverser_test, start_unreachable_from_any_other_node) {
    for (auto it = nodes.begin() + 1; it != nodes.end(); it++) {
        ASSERT_FALSE(nodes[0].reachable_from(nonterminal(*it)));
    }
}

TEST_F(grammar_traverser_test, deep_copy_is_deep_copy) {
    unordered_map<nonterminal, std::unique_ptr<grammar>> mut_copies = nodes[0].deep_copy();
    ASSERT_EQ(mut_copies.size(), nodes.size());

    unordered_set<const grammar*> mut_copies_set;
    for (const auto& [_, gram_ptr] : mut_copies) {
        mut_copies_set.insert(gram_ptr.get());
    }

    ASSERT_EQ(mut_copies_set.size(), nodes.size());
    
    for (const auto& [nont, copy_gram_ptr] : mut_copies) {
        assert_isomorphism(*nont, *copy_gram_ptr.get(), mut_copies);
    }
}