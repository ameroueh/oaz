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
using Model = oaz::nn::Model;
using Evaluator = NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;

using SharedModelPointer = std::shared_ptr<Model>;

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
		search->work();
		/* if (!work_done) { */
		/* 	/1* std::cout << "Forcing evaluation" << std::endl; *1/ */
		/* 	evaluator->forceEvaluation(); */
		/* } */
	}
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));

		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(model, 64));
		Game game;
		GameSearch(game, shared_evaluator_ptr, 1, 1);
	}
	
	TEST (Search, CheckSearchTree) {
		
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(model, 64));
		
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr, 1, 100);
		
		search_until_done(&search, shared_evaluator_ptr.get());
		
		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 100);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, CheckSearchTree) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(model, 16));
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr, 16, 1000);

		vector<std::thread> threads;
		for(size_t i=0; i!=2; ++i) {
			threads.push_back(std::thread(&search_until_done, &search, shared_evaluator_ptr.get()));
		}
		for(size_t i=0; i!=2; ++i) {
			threads[i].join();
		}

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 1000);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
	
	TEST (MultithreadedSearch, WithNoiseCheckSearchTree) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(model, 16));
		Game game;
		
		GameSearch search(game, shared_evaluator_ptr, 16, 1000, 0.25, 0.3);

		vector<std::thread> threads;
		for(size_t i=0; i!=2; ++i) {
			threads.push_back(std::thread(&search_until_done, &search, shared_evaluator_ptr.get()));
		}
		for(size_t i=0; i!=2; ++i) {
			threads[i].join();
		}

		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 1000);
		ASSERT_TRUE(checkSearchTree(search.getTreeRoot()));
	}
}
