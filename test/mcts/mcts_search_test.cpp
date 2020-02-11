#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class SelectNode_Default_Test; \
	friend class SelectNodes_Default_Test; \
	friend class EvaluateNodes_Default_Test; \
	friend class ExpandAndBackpropagateNodes_Default_Test; \
	friend class Search_Default_Test; \
	friend class Search_CheckSearchTree_Test;


#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"

#include <iostream>
#include <queue>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Board = ArrayBoard3D<float, 7, 6, 2>;
using Game = ConnectFour<Board>;
using Move = typename ConnectFour<Board>::move_t;
using Node = SearchNode<Game::move_t>;
using GamesContainer = vector<Game>;
using Evaluator = RandomEvaluator<Game, GamesContainer>;
using GameSearch = Search<Game, Evaluator>;

template <class Node>
bool checkSearchTree(Node* node) {
	size_t n_visits = node->getNVisits();
	bool correct = true;
	
	if(!node->isLeaf()) {
		bool correct_children = true;
		size_t n_children_visits = 0;

		for(size_t i=0; i!=node->getNChildren(); ++i) {
			Node* child = node->getChild(i);
			n_children_visits += child->getNVisits();
			correct_children &= checkSearchTree<Node>(child);

		}
		
		correct = (n_visits == (n_children_visits + 1)) && correct_children;
	}

	return correct; 
}

namespace oaz::mcts {
	TEST (Instantiation, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);

		Board board2;
		Game game(board2);

		GameSearch(1, game, &evaluator);
	}


	TEST (SelectNode, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 

		search.selectNode(0);
	}
	
	TEST (SelectNodes, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 

		search.selectNodes();

		ASSERT_TRUE(search.m_selection_q.empty());

		ASSERT_EQ(search.m_evaluation_q.front(), 0);
	}
	TEST (EvaluateNodes, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 

		search.selectNode(0);
		search.evaluateNodes();
	}
	
	TEST (ExpandAndBackpropagateNodes, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 

		search.selectNode(0);
		search.evaluateNodes();
		search.expandAndBackpropagateNodes();

		ASSERT_EQ(search.m_root.getNChildren(), 7);
		ASSERT_EQ(search.m_root.getNVisits(), 1);
	}
	
	TEST (Search, Default) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 
		
		for(int i=0; i!=100; ++i) {
			search.search(1);
			ASSERT_TRUE(search.m_game_evaluator->getGame(0) == game);
		}
	}
	
	TEST (Search, CheckSearchTree) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 
		
		search.search(100);

		ASSERT_TRUE(checkSearchTree(&search.m_root));
	}
	
	TEST (Search, GetTreeRoot) {
		Board board;
		GamesContainer games(1, Game(board));
		Evaluator evaluator(&games);
		
		Board board2;
		Game game(board2);
		
		GameSearch search (1, game, &evaluator); 

		search.search(1000);
		
		Node* root = search.getTreeRoot();		

		std::cout << serialiseTreeToJson<Move>(root) << std::endl;
	}
}
