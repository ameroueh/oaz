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
#include "oaz/utils/utils.hpp"
#include "oaz/thread_pool/dummy_task.hpp"

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
using TestEvaluator = SimulationEvaluator<Game>;
using GameSearch = MCTSSearch<Game>;

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		oaz::thread_pool::ThreadPool pool(1);
		std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(&pool));
		Game game;
		GameSearch(game, shared_evaluator_ptr.get(), &pool, 1, 1);
	}


	/* TEST (SelectNode, Default) { */
	/* 	oaz::thread_pool::ThreadPool pool(1); */
	/* 	std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(&pool)); */
	/* 	Game game; */
	/* 	GameSearch search(game, shared_evaluator_ptr.get(), &pool, 1, 1); */
	/* 	search.selectNode(0); */
	/* } */

	/* TEST (ExpandAndBackpropagateNode, Default) { */
	/* 	oaz::thread_pool::ThreadPool pool(1); */
	/* 	std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(&pool)); */
	/* 	Game game; */
	/* 	GameSearch search(game, shared_evaluator_ptr.get(), &pool, 1, 1); */
		
	/* 	search.selectNode(0); */
	/* 	search.expandAndBackpropagateNode(0); */
	/* } */
	
	/* TEST (Search, CheckSearchTree) { */
	/* 	oaz::thread_pool::ThreadPool pool(1); */
	/* 	std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(&pool)); */
	/* 	Game game; */
	/* 	GameSearch search (game, shared_evaluator_ptr.get(), &pool, 1, 100); */ 

	/* 	search.search(); */

	/* 	ASSERT_EQ(search.getTreeRoot()->getNVisits(), 100); */
	/* 	ASSERT_TRUE(checkSearchTree(&search.m_root)); */
	/* } */
	
	/* TEST (MultithreadedSearch, CheckSearchTree) { */
	/* 	oaz::thread_pool::ThreadPool pool(2); */
	/* 	std::shared_ptr<TestEvaluator> shared_evaluator_ptr(new TestEvaluator(&pool)); */
	/* 	Game game; */
	/* 	GameSearch search (game, shared_evaluator_ptr.get(), &pool, 2, 1000); */ 
	/* 	search.search(); */

	/* 	Node* root = search.getTreeRoot(); */

	/* 	ASSERT_EQ(root->getNVisits(), 1000); */
	/* 	ASSERT_TRUE(checkSearchTree(root)); */
	/* } */
}
