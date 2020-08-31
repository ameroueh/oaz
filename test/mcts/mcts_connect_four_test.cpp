#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/mcts/mcts_search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

#include <iostream>
#include <queue>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = ConnectFour;
using Move = typename Game::Move;
using Node = SearchNode<Game::Move>;
using GameEvaluator = SimulationEvaluator<Game>;
using GameSearch = MCTSSearch<Game>;
using SharedEvaluatorPointer = std::shared_ptr<GameEvaluator>;

TEST (ForcedMoves, Scenario1) {
	oaz::thread_pool::ThreadPool pool(8);
	SharedEvaluatorPointer evaluator(new GameEvaluator(&pool));
	Game game;

	game.playFromString("5443233212"); // Expect best move to be 2

	GameSearch search (game, evaluator.get(), &pool, 16, 10000); 

	search.search();

	ASSERT_EQ(search.getBestMove(), 2);
}

TEST (ForcedMoves, Scenario2) {
	
	oaz::thread_pool::ThreadPool pool(8);
	SharedEvaluatorPointer evaluator(new GameEvaluator(&pool));
	Game game;
	
	game.playFromString("4330011115"); // Expect best move to be 2

	GameSearch search (game, evaluator.get(), &pool, 8, 10000); 

	search.search();

	ASSERT_EQ(search.getBestMove(), 2);
}

TEST (ForcedMoves, Scenario3) {
	
	oaz::thread_pool::ThreadPool pool(8);
	SharedEvaluatorPointer evaluator(new GameEvaluator(&pool));
	Game game;
	
	game.playFromString("433001111"); // Expect best move to be 2

	GameSearch search (game, evaluator.get(), &pool, 16, 10000); 

	search.search();
	
	ASSERT_EQ(search.getBestMove(), 2);
}
