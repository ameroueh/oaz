#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tensorflow/core/framework/tensor.h"
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

void createAndPerformSearch(std::shared_ptr<Evaluator> shared_evaluator_ptr, SearchPool* search_pool, size_t n_iterations) {
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 16, n_iterations);
		search_pool->performSearch(&search);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		
		SearchPool(shared_evaluator_ptr, 1.);
	}
	
	TEST (SingleSearch, Singlethreaded) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 1, 100);
		SearchPool search_pool(shared_evaluator_ptr, 1.);
		
		search_pool.performSearch(&search);
	}
	
	TEST (SingleSearch, Multithreaded) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 16, 1000);
		SearchPool search_pool(shared_evaluator_ptr, 2.);
		
		search_pool.performSearch(&search);
	}
	
	TEST (Multisearch, Singlethreaded) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		SearchPool search_pool(shared_evaluator_ptr, 0.2);
		
		
		std::vector<std::thread> t;
		for(size_t i=0; i!=2; ++i)
			t.push_back(std::thread(createAndPerformSearch, shared_evaluator_ptr, &search_pool, 5000));
		
		for(size_t i=0; i!=2; ++i)
			t[i].join();
	}
	
	TEST (Multisearch, Multithreaded) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		SearchPool search_pool(shared_evaluator_ptr, 2.);
		
		std::vector<std::thread> t;
		for(size_t i=0; i!=2; ++i)
			t.push_back(std::thread(createAndPerformSearch, shared_evaluator_ptr, &search_pool, 5000));
		
		for(size_t i=0; i!=2; ++i)
			t[i].join();
	}
}
