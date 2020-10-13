#include <algorithm>
#include <string>

#include "oaz/games/connect_four.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::games;
using namespace testing;

TEST (InstantiationTest, Default) {
	ConnectFour game;
}

TEST (GetAvailableMoves, Default) {
	ConnectFour game;
	std::vector<size_t> available_moves;
	game.GetAvailableMoves(available_moves);
	ASSERT_THAT(available_moves, ElementsAre(0, 1, 2, 3, 4, 5, 6));
}

TEST (PlayTest, VerticalVictory) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("0103040");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(1, game.GetScore());
}

TEST (PlayTest, VerticalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("60103040");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(-1, game.GetScore());
}

TEST (PlayTest, HorizontalVictory) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("051625364");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(1, game.GetScore());
}

TEST (PlayTest, HorizontalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("6051625364");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(-1, game.GetScore());
}


TEST (PlayTest, FirstDiagonalVictory) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("12234334544");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(1, game.GetScore());
}

TEST (PlayTest, FirstDiagonalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("612234334544");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(-1, game.GetScore());
}


TEST (PlayTest, SecondDiagonalVictory) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("54432332122");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(1, game.GetScore());
}

TEST (PlayTest, SecondDiagonalVictoryPlayer2) {
	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("654432332122");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(-1, game.GetScore());
}

TEST (PlayTest, TieTest) {

	ConnectFour game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("021302130213465640514455662233001144552636");
	std::vector<size_t> available_moves;
	game.GetAvailableMoves(available_moves);
	ASSERT_THAT(available_moves, ElementsAre());
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(0, game.GetScore());
}

TEST (Clone, Default) {
	ConnectFour game;
	game.PlayFromString("021302130213465640514455662233001144552636");
	std::unique_ptr<Game> clone = game.Clone();
	ConnectFour* clone_ptr = dynamic_cast<ConnectFour*>(clone.get());
	ASSERT_TRUE(game == *clone_ptr);
}

TEST (GetCurrentPlayer, Default) {
	ConnectFour game;
	ASSERT_EQ(game.GetCurrentPlayer(), 0);
	game.PlayMove(0);
	ASSERT_EQ(game.GetCurrentPlayer(), 1);
}

TEST (ClassMethods, Default) {
	ConnectFour game;
	Game* game_ptr = &game;
	ASSERT_EQ(game_ptr->ClassMethods().GetMaxNumberOfMoves(), 7);
	ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[0], 6);
	ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[1], 7);
	ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[2], 2);
}

TEST (WriteStateToTensorMemory, Default) {
	ConnectFour game;
	game.PlayMove(0);
	game.PlayMove(5);
	boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
	game.WriteStateToTensorMemory(tensor.origin());
	for(size_t i=0; i!=6; ++i)
		for(size_t j=0; j!=7; ++j)
			if(i==0 && j==0)
				ASSERT_EQ(tensor[i][j][0], 1.);
			else ASSERT_EQ(tensor[i][j][0], 0.);
	for(size_t i=0; i!=6; ++i)
		for(size_t j=0; j!=7; ++j)
			if(i==0 && j==5)
				ASSERT_EQ(tensor[i][j][1], 1.);
			else ASSERT_EQ(tensor[i][j][1], 0.);
}
