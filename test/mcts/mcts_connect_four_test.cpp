#include "gtest/gtest.h"
#include "gmock/gmock.h"

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

void playFromString(Game* game, std::string sMoves) {
	for(char& c : sMoves)
		game->playMove(c - '0');
}

TEST (ForcedMoves, Scenario1) {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);
	
	Board board2;
	Game game(board2);

	playFromString(&game, "5443233212"); // Expect best move to be 2

	GameSearch search (1, game, &evaluator); 

	search.search(1000);

	ASSERT_EQ(search.getBestMove(), 2);
}

TEST (ForcedMoves, Scenario2) {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);
	
	Board board2;
	Game game(board2);

	playFromString(&game, "4330011115"); // Expect best move to be 2

	GameSearch search (1, game, &evaluator); 
	
	search.search(10000);
	
	ASSERT_EQ(search.getBestMove(), 2);
}

TEST (ForcedMoves, Scenario3) {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);
	
	Board board2;
	Game game(board2);

	playFromString(&game, "433001111"); // Expect best move to be 2

	GameSearch search (1, game, &evaluator); 

	game.playMove(2);

	search.search(10000);

	ASSERT_EQ(search.getBestMove(), 2);
}
