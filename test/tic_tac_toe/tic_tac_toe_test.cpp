#include <algorithm>
#include <string>

#include "oaz/games/tic_tac_toe.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::games;
using namespace testing;

void playFromString(TicTacToe* game, std::string sMoves) {
	for(char& c : sMoves)
		game->playMove(c - '0');
}

TEST (InstantiationTest, Default) {
	TicTacToe game;
}

TEST (InstantiationTest, AvailableMoves) {
	TicTacToe game;
	ASSERT_THAT(*(game.availableMoves()), ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8));
}


TEST (ResetTest, Default) {
	TicTacToe game;
	game.reset();
}


TEST (PlayTest, DoUndo) {
	TicTacToe game;
	TicTacToe game2;

	auto available_moves = *(game.availableMoves());
	
	for(int i=0; i!=available_moves.size(); ++i) {
		auto move_to_play = available_moves[i];
		game2.playMove(move_to_play);
		game2.undoMove(move_to_play);
		ASSERT_TRUE(game == game2);
	}
}

TEST (PlayTest, VerticalVictory) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "03142");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, VerticalVictoryPlayer2) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "031465");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, HorizontalVictory) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "01326");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, HorizontalVictoryPlayer2) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "510467");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, FirstDiagonalVictory) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "01428");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, FirstDiagonalVictoryPlayer2) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "301478");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, SecondDiagonalVictory) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "21456");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, SecondDiagonalVictoryPlayer2) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "023416");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, TieTest) {
	TicTacToe game;
	ASSERT_TRUE(~game.Finished());
	playFromString(
		&game, 
		"036451287"
	);
	ASSERT_THAT(*(game.availableMoves()), ElementsAre());
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(0, game.score());
}

TEST (CopyTest, Default) {
	TicTacToe game;
	playFromString(
		&game, 
		"036451287"
	);
	TicTacToe game2(game);
	ASSERT_TRUE(game == game2);
}

TEST (GetCurrentPlayer, Default) {
	TicTacToe game;
	ASSERT_EQ(game.getCurrentPlayer(), 0);
	game.playMove(0);
	ASSERT_EQ(game.getCurrentPlayer(), 1);
}

TEST (Set, Default) {
	TicTacToe game;
	playFromString(&game, "0123");
	
	TicTacToe game2;
	game2.set(game);

	ASSERT_TRUE(game == game2);	
}
