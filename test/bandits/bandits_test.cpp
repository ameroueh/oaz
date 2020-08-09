#include <algorithm>
#include <string>

#include "oaz/games/bandits.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::games;
using namespace testing;

TEST (InstantiationTest, Default) {
	Bandits game;
}

TEST (InstantiationTest, AvailableMoves) {
	Bandits game;
	ASSERT_THAT(*(game.availableMoves()), ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}


TEST (ResetTest, Default) {
	Bandits game;
	game.reset();
}


TEST (PlayTest, DoUndo) {
	Bandits game;
	Bandits game2;

	auto available_moves = *(game.availableMoves());
	
	for(int i=0; i!=available_moves.size(); ++i) {
		auto move_to_play = available_moves[i];
		game2.playMove(move_to_play);
		game2.undoMove(move_to_play);
		ASSERT_TRUE(game == game2);
	}
}

TEST (PlayTest, Victory) {
	Bandits game;
	ASSERT_TRUE(~game.Finished());
	game.playFromString("3");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1., game.score());
}

TEST (PlayTest, Victory2) {
	Bandits game;
	ASSERT_TRUE(~game.Finished());
	game.playFromString("2");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1., game.score());
}

TEST (CopyTest, Default) {
	Bandits game;
	game.playFromString("6");
	Bandits game2(game);
	ASSERT_TRUE(game == game2);
}

TEST (GetCurrentPlayer, Default) {
	Bandits game;
	ASSERT_EQ(game.getCurrentPlayer(), 0);
	game.playMove(0);
	ASSERT_EQ(game.getCurrentPlayer(), 1);
}

TEST (Set, Default) {
	Bandits game;
	game.playFromString("2");
	
	Bandits game2;
	game2.set(game);

	ASSERT_TRUE(game == game2);	
}
