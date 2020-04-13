#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class WaitingForEvaluation_Default_Test;

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
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
using Move = typename Game::Move;
using Node = SearchNode<Move>;
using Evaluator = NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;

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

void search_until_done(GameSearch* search, Evaluator* evaluator) {
	bool work_done = false;
	while(!search->done()) {
		/* std::cout << "Working" << std::endl; */
		work_done = search->work();
		if (!work_done) {
			/* std::cout << "Forcing evaluation" << std::endl; */
			evaluator->forceEvaluation();
		}
	}
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		Game game;
		GameSearch(game, shared_evaluator_ptr, 1, 1);
	}
	
	TEST (WaitingForEvaluation, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 1, 1);
		
		ASSERT_FALSE(search.waitingForEvaluation());
		search.selectNode(0);
		ASSERT_TRUE(search.waitingForEvaluation());
		shared_evaluator_ptr->forceEvaluation();
		ASSERT_FALSE(search.waitingForEvaluation());
	}
	
	

	TEST (Search, CheckSearchTree) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 1, 100);
		
		search_until_done(&search, shared_evaluator_ptr.get());
		
		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 100);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, CheckSearchTree) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(16));
		shared_evaluator_ptr->load_model("model");
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr, 16, 10000);

		vector<std::thread> threads;
		for(size_t i=0; i!=2; ++i) {
			threads.push_back(std::thread(&search_until_done, &search, shared_evaluator_ptr.get()));
		}
		for(size_t i=0; i!=2; ++i) {
			threads[i].join();
		}

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 10000);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
}
