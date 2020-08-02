#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class SelectNode_Default_Test; \
	friend class ExpandAndBackpropagateNode_Default_Test; \
	friend class SelectNodes_Default_Test; \
	friend class EvaluateNodes_Default_Test; \
	friend class ExpandAndBackpropagateNodes_Default_Test; \
	friend class Search_Default_Test; \
	friend class MultithreadedSearch_CheckSearchTree_Test; \
	friend class Search_CheckSearchTree_Test; \
	friend class WaitingForEvaluation_Default_Test;


#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/mcts/mcts_search.hpp"
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
using Node = SearchNode<Game::Move>;
using Evaluator = SimulationEvaluator<Game, SafeQueueNotifier>;
using GameSearch = MCTSSearch<Game, Evaluator>;

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

void search_until_done(GameSearch* search) {
	while(!search->done()) 
		search->work();
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
		Game game;
		GameSearch(game, shared_evaluator_ptr, 1, 1);
	}


	TEST (SelectNode, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 1, 1);
		search.selectNode(0);
	}

	TEST (ExpandAndBackpropagateNode, Default) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
		Game game;
		GameSearch search(game, shared_evaluator_ptr, 1, 1);
		
		search.selectNode(0);
		search.expandAndBackpropagateNode(0);
	}
	
	TEST (Search, CheckSearchTree) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
		Game game;
		GameSearch search (game, shared_evaluator_ptr, 1, 100); 
	
		while(!search.done())
			search.work();
		
		ASSERT_EQ(search.getTreeRoot()->getNVisits(), 100);
		ASSERT_TRUE(checkSearchTree(&search.m_root));
	}
	
	TEST (MultithreadedSearch, CheckSearchTree) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
		Game game;
		GameSearch search (game, shared_evaluator_ptr, 2, 1000); 

		std::vector<std::thread> threads;
		for(size_t i=0; i!=2; ++i) 
			threads.push_back(std::thread(&search_until_done, &search));

		for(size_t i=0; i!=2; ++i)
			threads[i].join();
		
		Node* root = search.getTreeRoot();

		ASSERT_EQ(root->getNVisits(), 1000);
		ASSERT_TRUE(checkSearchTree(root));
	}
}
