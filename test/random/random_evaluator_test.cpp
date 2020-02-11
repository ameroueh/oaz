#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"

#include <vector>

using namespace std;

using Board = ArrayBoard3D<float, 7, 6, 2>;
using Game = ConnectFour<Board>;
using GamesContainer = vector<Game>;
using Evaluator = RandomEvaluator<Game, GamesContainer>;


TEST (Instantiation, Default) {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);
}

TEST (GetGame, Default) {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);
	Game& game2 = evaluator.getGame(0);
	ASSERT_TRUE(games[0] == game2);
	ASSERT_EQ(&games[0], &game2);
}

TEST (Evaluate, Default) {
	Board board;
	GamesContainer games(1, Game(board));
	
	Evaluator evaluator(&games);
	Board board2;

	Game& game = evaluator.getGame(0);
	Game game2(game, board2);

	evaluator.evaluate();

	ASSERT_TRUE(game2 == evaluator.getGame(0));

	game.playMove(0);
	game2.playMove(0);

	evaluator.evaluate();

	ASSERT_TRUE(game2 == evaluator.getGame(0));
}
