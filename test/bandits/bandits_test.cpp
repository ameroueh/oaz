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
	std::vector<size_t> available_moves;
	game.GetAvailableMoves(available_moves);
	ASSERT_THAT(available_moves, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST (PlayTest, Victory) {
	Bandits game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("3");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(-1., game.GetScore());
}

TEST (PlayTest, Victory2) {
	Bandits game;
	ASSERT_FALSE(game.IsFinished());
	game.PlayFromString("2");
	ASSERT_TRUE(game.IsFinished());
	ASSERT_EQ(1., game.GetScore());
}

TEST (Clone, Default) {
	Bandits game;
	game.PlayFromString("6");
	std::unique_ptr<Game> clone = game.Clone();
	Bandits* clone_ptr = dynamic_cast<Bandits*>(clone.get());
	ASSERT_TRUE(game == *clone_ptr);
}

TEST (GetCurrentPlayer, Default) {
	Bandits game;
	ASSERT_EQ(game.GetCurrentPlayer(), 0);
	game.PlayMove(0);
	ASSERT_EQ(game.GetCurrentPlayer(), 1);
}

TEST (ClassMethods, Default) {
	Bandits game;
	Game* game_ptr = &game;
	ASSERT_EQ(game_ptr->ClassMethods().GetMaxNumberOfMoves(), 10);
	ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[0], 10);
}

TEST (WriteStateToTensorMemory, Default) {
	Bandits game;
	game.PlayMove(0);
	boost::multi_array<float, 1> tensor(boost::extents[10]);
	game.WriteStateToTensorMemory(tensor.origin());
	for(size_t i=0; i!=10; ++i)
		if(i==0)
			ASSERT_EQ(tensor[i], 1.);
		else ASSERT_EQ(tensor[i], 0.);
}
