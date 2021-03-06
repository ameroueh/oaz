#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class WaitingForEvaluation_Default_Test;

#include "oaz/thread_pool/thread_pool.hpp"
#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/search.hpp" 
#include "oaz/mcts/selection.hpp"
/* #include "oaz/mcts/search_node_serialisation.hpp" */
#include "oaz/utils/utils.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::unique_ptr<tensorflow::Session> session(
			oaz::nn::CreateSessionAndLoadGraph(
				"frozen_model.pb"
			)
		);
		auto model = oaz::nn::CreateModel(
			session.get(), 
			"value",
			"policy"
		);
		auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
		std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
			new oaz::nn::NNEvaluator(
				model, 
				nullptr,
				pool, 
				{6, 7, 2}, 
				64
			)
		);
		ConnectFour game;
		AZSelector selector;
		oaz::mcts::Search(
			game, 
			selector, 
			evaluator,
			pool, 
			1, 
			1
		);
	}
	
	TEST (Search, CheckSearchTree) {
		std::unique_ptr<tensorflow::Session> session(
			oaz::nn::CreateSessionAndLoadGraph(
				"frozen_model.pb"
			)
		);
		auto model = oaz::nn::CreateModel(
			session.get(), 
			"value",
			"policy"
		);
		auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
		std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
			new oaz::nn::NNEvaluator(
				model, 
				nullptr,
				pool, 
				{6, 7, 2}, 
				1
			)
		);
		ConnectFour game;
		AZSelector selector;
		oaz::mcts::Search(
			game, 
			selector, 
			evaluator,
			pool, 
			1, 
			100
		);
		Search search(game, selector, evaluator, pool, 1, 100);
		
		ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 100);
		ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
	}
	
	TEST (MultithreadedSearch, CheckSearchTree) {
		std::unique_ptr<tensorflow::Session> session(
			oaz::nn::CreateSessionAndLoadGraph(
				"frozen_model.pb"
			)
		);
		auto model = oaz::nn::CreateModel(
			session.get(), 
			"value",
			"policy"
		);
		auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(2);
		std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
			new oaz::nn::NNEvaluator(
				model, 
				nullptr,
				pool, 
				{6, 7, 2}, 
				8	
			)
		);
		ConnectFour game;
		AZSelector selector;
		oaz::mcts::Search search(
			game, 
			selector, 
			evaluator,
			pool, 
			16, 
			1000
		);
		ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 1000);
		ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
	}
	
	TEST (MultithreadedSearch, WithNoiseCheckSearchTree) {
		std::unique_ptr<tensorflow::Session> session(
			oaz::nn::CreateSessionAndLoadGraph(
				"frozen_model.pb"
			)
		);
		auto model = oaz::nn::CreateModel(
			session.get(), 
			"value",
			"policy"
		);
		auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(2);
		std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
			new oaz::nn::NNEvaluator(
				model, 
				nullptr,
				pool, 
				{6, 7, 2}, 
				8	
			)
		);
		ConnectFour game;
		AZSelector selector;
		oaz::mcts::Search search(
			game, 
			selector, 
			evaluator,
			pool, 
			16, 
			1000,
			0.25,
			0.3
		);
		ASSERT_EQ(search.GetTreeRoot()->GetNVisits(), 1000);
		ASSERT_TRUE(CheckSearchTree(search.GetTreeRoot().get()));
	}
	
	TEST (MultithreadedSearch, Performance) {
		std::unique_ptr<tensorflow::Session> session(
			oaz::nn::CreateSessionAndLoadGraph(
				"frozen_model.pb"
			)
		);
		auto model = oaz::nn::CreateModel(
			session.get(), 
			"value",
			"policy"
		);
		auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(4);
		std::shared_ptr<oaz::nn::NNEvaluator> evaluator(
			new oaz::nn::NNEvaluator(
				model, 
				nullptr,
				pool, 
				{6, 7, 2}, 
				50	
			)
		);
		ConnectFour game;
		AZSelector selector;
		oaz::mcts::Search search(
			game, 
			selector, 
			evaluator,
			pool, 
			200, 
			300000
		);
	}
}
