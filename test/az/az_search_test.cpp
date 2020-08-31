#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class WaitingForEvaluation_Default_Test;

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"
/* #include "oaz/games/connect_four.hpp" */
#include "oaz/games/bandits.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"
#include "oaz/utils/utils.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = Bandits;
using Move = typename Game::Move;
using Node = SearchNode<Move>;
using Model = oaz::nn::Model;
using TestEvaluator = NNEvaluator<Game>;
using GameSearch = AZSearch<Game>;
using SharedModelPointer = std::shared_ptr<Model>;

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		oaz::thread_pool::ThreadPool pool(1);
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));

		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(
			new TestEvaluator(model, &pool, 64)
		);
		Game game;
		GameSearch(game, shared_evaluator_ptr.get(), &pool, 1, 1);
	}
	
	TEST (Search, CheckSearchTree) {
		oaz::thread_pool::ThreadPool pool(1);
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(model, &pool, 1));
		
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr.get(), &pool, 1, 100);
		
		search.search();	

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 100);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, CheckSearchTree) {
		oaz::thread_pool::ThreadPool pool(2);
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(model, &pool, 1));
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr.get(), &pool, 16, 1000);
		search.search();

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 1000);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, WithNoiseCheckSearchTree) {
		oaz::thread_pool::ThreadPool pool(2);
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(model, &pool, 1));
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr.get(), &pool, 16, 1000, 0.25, 0.3);
		
		search.search();

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 1000);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, Performance) {
		oaz::thread_pool::ThreadPool pool(8);
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(model, &pool, 100));
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr.get(), &pool, 200, 300000);
		search.search();
	}
	
}
