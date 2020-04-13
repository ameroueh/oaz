#include <algorithm>
#include <string>

#include "oaz/games/connect_four.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::games;
using namespace testing;

void playFromString(ConnectFour* game, std::string sMoves) {
	for(char& c : sMoves)
		game->playMove(c - '0');
}

TEST (InstantiationTest, Default) {
	ConnectFour game;
}

TEST (InstantiationTest, AvailableMoves) {
	ConnectFour game;
	ASSERT_THAT(*(game.availableMoves()), ElementsAre(0, 1, 2, 3, 4, 5, 6));
}


TEST (ResetTest, Default) {
	ConnectFour game;
	game.reset();
}


TEST (PlayTest, DoUndo) {
	ConnectFour game;
	ConnectFour game2;

	auto available_moves = *(game.availableMoves());
	
	for(int i=0; i!=available_moves.size(); ++i) {
		auto move_to_play = available_moves[i];
		game2.playMove(move_to_play);
		game2.undoMove(move_to_play);
		ASSERT_TRUE(game == game2);
	}
}

TEST (PlayTest, VerticalVictory) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "0103040");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, VerticalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "60103040");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, HorizontalVictory) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "051625364");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, HorizontalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "6051625364");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, FirstDiagonalVictory) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "12234334544");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, FirstDiagonalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "612234334544");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, SecondDiagonalVictory) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "54432332122");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, SecondDiagonalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "654432332122");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, TieTest) {
	ConnectFour game;
	ASSERT_TRUE(~game.Finished());
	playFromString(
		&game, 
		"021302130213465640514455662233001144552636"
	);
	ASSERT_THAT(*(game.availableMoves()), ElementsAre());
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(0, game.score());
}

TEST (CopyTest, Default) {
	ConnectFour game;
	playFromString(
		&game, 
		"021302130213465640514455662233001144552636"
	);
	ConnectFour game2(game);
	ASSERT_TRUE(game == game2);
}

TEST (GetCurrentPlayer, Default) {
	ConnectFour game;
	ASSERT_EQ(game.getCurrentPlayer(), 0);
	game.playMove(0);
	ASSERT_EQ(game.getCurrentPlayer(), 1);
}

TEST (Set, Default) {
	ConnectFour game;
	playFromString(&game, "0123");
	
	ConnectFour game2;
	game2.set(game);

	ASSERT_TRUE(game == game2);	
}
