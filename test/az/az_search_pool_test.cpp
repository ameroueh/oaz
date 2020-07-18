#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = ConnectFour;
using Move = typename ConnectFour::Move;
using Node = SearchNode<Game::Move>;
using Evaluator = NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;
using SearchPool = AZSearchPool<Game, Evaluator>;

using SharedModelPointer = std::shared_ptr<Model>;
using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;


template <class Node>
bool checkSearchTree(Node* node) {
	size_t n_visits = node->getNVisits();
	bool overall_correct = true;
	
	if(!node->isLeaf()) {
		bool correct_children = true;
		size_t n_children_visits = 0;

		for(size_t i=0; i!=node->getNChildren(); ++i) {
			Node* child = node->getChild(i);
			n_children_visits += child->getNVisits();
			correct_children &= checkSearchTree<Node>(child);

		}
		
		if(!correct_children)
			std::cout << "Incorrect children at " << node << std::endl;
		bool correct = (n_visits == (n_children_visits + 1));
		if(!correct)
			std::cout << "Incorrect node at " << node << "; n_children_visits " << n_children_visits << "; n_visits " << n_visits << std::endl;

		overall_correct = correct && correct_children;
	}

	return overall_correct; 
}

void createAndPerformSearch(SharedEvaluatorPointer evaluator, SearchPool* search_pool, size_t n_iterations) {
		Game game;
		GameSearch search(game, evaluator, 4, n_iterations);
		search_pool->performSearch(&search);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SearchPool search_pool(evaluator, 1);
	}
	
	TEST (SingleSearch, Singlethreaded) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SearchPool search_pool(evaluator, 1);
		
		Game game;
		GameSearch search(game, evaluator, 1, 100);
		
		search_pool.performSearch(&search);
	}
	
	TEST (SingleSearch, Multithreaded) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SearchPool search_pool(evaluator, 2);
		
		Game game;
		GameSearch search(game, evaluator, 16, 1000);
		
		search_pool.performSearch(&search);
	}
	
	TEST (Multisearch, Singlethreaded) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 2)
		);
		SearchPool search_pool(evaluator, 1);
		
		std::vector<std::thread> t;
		for(size_t i=0; i!=2; ++i)
			t.push_back(std::thread(createAndPerformSearch, evaluator, &search_pool, 100));
		
		for(size_t i=0; i!=2; ++i)
			t[i].join();
	}
	
	TEST (Multisearch, Multithreaded) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 4)
		);
		SearchPool search_pool(evaluator, 4);
		
		std::vector<std::thread> t;
		for(size_t i=0; i!=2; ++i)
			t.push_back(std::thread(createAndPerformSearch, evaluator, &search_pool, 100));
		
		for(size_t i=0; i!=2; ++i)
			t[i].join();
	}
}
