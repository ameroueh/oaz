#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define TEST_FRIENDS friend class WaitingForEvaluation_Default_Test;

#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/search.hpp"
#include "oaz/mcts/selection.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/thread_pool/thread_pool.hpp"
#include "tensorflow/core/framework/tensor.h"
/* #include "oaz/mcts/search_node_serialisation.hpp" */
#include <iostream>
#include <queue>
#include <thread>
#include <vector>

#include "oaz/utils/utils.hpp"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

namespace oaz::mcts {
TEST(Instantiation, Default) {
  std::unique_ptr<tensorflow::Session> session(
      oaz::nn::CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = oaz::nn::CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
      new oaz::nn::NNEvaluator(model, nullptr, pool, {6, 7, 2}, 64));
  ConnectFour game;
  std::shared_ptr<Selector> selector = std::make_shared<AZSelector>();
  auto player_search_properties = {
    PlayerSearchProperties(evaluator, selector),
    PlayerSearchProperties(evaluator, selector)
  };
  oaz::mcts::Search(game, player_search_properties, pool, 1, 1);
}

TEST(Search, CheckSearchTree) {
  std::unique_ptr<tensorflow::Session> session(
      oaz::nn::CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = oaz::nn::CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
      new oaz::nn::NNEvaluator(model, nullptr, pool, {6, 7, 2}, 1));
  ConnectFour game;
  std::shared_ptr<Selector> selector = std::make_shared<AZSelector>();
  auto player_search_properties = {
    PlayerSearchProperties(evaluator, selector),
    PlayerSearchProperties(evaluator, selector)
  };
  oaz::mcts::Search(game, player_search_properties, pool, 1, 100);
  Search search(game, player_search_properties, pool, 1, 100);

  ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 100);
  ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
}

TEST(MultithreadedSearch, CheckSearchTree) {
  std::unique_ptr<tensorflow::Session> session(
      oaz::nn::CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = oaz::nn::CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(2);
  std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
      new oaz::nn::NNEvaluator(model, nullptr, pool, {6, 7, 2}, 8));
  ConnectFour game;
  std::shared_ptr<Selector> selector = std::make_shared<AZSelector>();
  auto player_search_properties = {
    PlayerSearchProperties(evaluator, selector),
    PlayerSearchProperties(evaluator, selector)
  };
  oaz::mcts::Search search(game, player_search_properties, pool, 16, 1000);
  ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 1000);
  ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
}

TEST(MultithreadedSearch, WithNoiseCheckSearchTree) {
  std::unique_ptr<tensorflow::Session> session(
      oaz::nn::CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = oaz::nn::CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(2);
  std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
      new oaz::nn::NNEvaluator(model, nullptr, pool, {6, 7, 2}, 8));
  ConnectFour game;
  std::shared_ptr<Selector> selector = std::make_shared<AZSelector>();
  auto player_search_properties = {
    PlayerSearchProperties(evaluator, selector),
    PlayerSearchProperties(evaluator, selector)
  };
  oaz::mcts::Search search(game, player_search_properties, pool, 16, 1000, 0.25,
                           0.3);
  ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 1000);
  ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
}

TEST(MultithreadedSearch, Performance) {
  std::unique_ptr<tensorflow::Session> session(
      oaz::nn::CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = oaz::nn::CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(4);
  std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
      new oaz::nn::NNEvaluator(model, nullptr, pool, {6, 7, 2}, 50));
  ConnectFour game;
  std::shared_ptr<Selector> selector = std::make_shared<AZSelector>();
  auto player_search_properties = {
    PlayerSearchProperties(evaluator, selector),
    PlayerSearchProperties(evaluator, selector)
  };
  oaz::mcts::Search search(game, player_search_properties, pool, 200, 300000);
}
}  // namespace oaz::mcts
