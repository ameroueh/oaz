#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define TEST_FRIENDS                                     \
  friend class SelectNode_Default_Test;                  \
  friend class ExpandAndBackpropagateNode_Default_Test;  \
  friend class SelectNodes_Default_Test;                 \
  friend class EvaluateNodes_Default_Test;               \
  friend class ExpandAndBackpropagateNodes_Default_Test; \
  friend class Search_Default_Test;                      \
  friend class MultithreadedSearch_CheckSearchTree_Test; \
  friend class Search_CheckSearchTree_Test;              \
  friend class WaitingForEvaluation_Default_Test;

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/search.hpp"
#include "oaz/mcts/selection.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/utils/utils.hpp"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

namespace oaz::mcts {
TEST(Instantiation, Default) {
  auto pool = make_shared<oaz::thread_pool::ThreadPool>(1);
  auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
  UCTSelector selector;
  ConnectFour game;
  Search(game, selector, evaluator, pool, 1, 1);
}

TEST(Search, CheckSearchTree) {
  auto pool = make_shared<oaz::thread_pool::ThreadPool>(1);
  auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
  ConnectFour game;
  UCTSelector selector;
  Search search(game, selector, evaluator, pool, 1, 100);
  auto tree_root = search.GetTreeRoot();
  ASSERT_EQ(tree_root->GetNVisits(), 100);
  ASSERT_TRUE(CheckSearchTree(tree_root.get()));
}

TEST(MultithreadedSearch, CheckSearchTree) {
  auto pool = make_shared<oaz::thread_pool::ThreadPool>(2);
  auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
  ConnectFour game;
  UCTSelector selector;
  Search search(game, selector, evaluator, pool, 2, 1000);
  auto tree_root = search.GetTreeRoot();
  ASSERT_EQ(tree_root->GetNVisits(), 1000);
  ASSERT_TRUE(CheckSearchTree(tree_root.get()));
}
}  // namespace oaz::mcts
