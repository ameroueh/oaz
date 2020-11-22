#include "oaz/games/tic_tac_toe.hpp"

#include <algorithm>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::games;
using namespace testing;

TEST(InstantiationTest, Default) {
    TicTacToe game;
}

TEST(GetAvailableMoves, Default) {
    TicTacToe game;
    std::vector<size_t> available_moves;
    game.GetAvailableMoves(available_moves);
    ASSERT_THAT(available_moves, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(PlayTest, VerticalVictory) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("03142");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, VerticalVictoryPlayer2) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("031465");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, HorizontalVictory) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("01326");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, HorizontalVictoryPlayer2) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("510467");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, FirstDiagonalVictory) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("01428");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, FirstDiagonalVictoryPlayer2) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("301478");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, SecondDiagonalVictory) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("21456");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, SecondDiagonalVictoryPlayer2) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("023416");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, TieTest) {
    TicTacToe game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("036451287");
    std::vector<size_t> available_moves;
    game.GetAvailableMoves(available_moves);
    ASSERT_THAT(available_moves, ElementsAre());
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(0, game.GetScore());
}

TEST(Clone, Default) {
    TicTacToe game;
    game.PlayFromString("036451287");
    std::unique_ptr<Game> clone = game.Clone();
    TicTacToe* clone_ptr = dynamic_cast<TicTacToe*>(clone.get());
    ASSERT_TRUE(game == *clone_ptr);
}

TEST(GetCurrentPlayer, Default) {
    TicTacToe game;
    ASSERT_EQ(game.GetCurrentPlayer(), 0);
    game.PlayMove(0);
    ASSERT_EQ(game.GetCurrentPlayer(), 1);
}

TEST(ClassMethods, Default) {
    TicTacToe game;
    Game* game_ptr = &game;
    ASSERT_EQ(game_ptr->ClassMethods().GetMaxNumberOfMoves(), 9);
    ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[0], 3);
    ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[1], 3);
    ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[2], 2);
}

TEST(WriteStateToTensorMemory, Default) {
    TicTacToe game;
    game.PlayMove(0);
    game.PlayMove(5);
    boost::multi_array<float, 3> tensor(boost::extents[3][3][2]);
    game.WriteStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 3; ++i)
        for (size_t j = 0; j != 3; ++j)
            if (i == 0 && j == 0)
                ASSERT_EQ(tensor[i][j][0], 1.);
            else
                ASSERT_EQ(tensor[i][j][0], 0.);
    for (size_t i = 0; i != 3; ++i)
        for (size_t j = 0; j != 3; ++j)
            if (i == 2 && j == 1)
                ASSERT_EQ(tensor[i][j][1], 1.);
            else
                ASSERT_EQ(tensor[i][j][1], 0.);
}

TEST(WriteCanonicalStateToTensorMemory, Default) {
    TicTacToe game;
    game.PlayMove(0);
    boost::multi_array<float, 3> tensor(boost::extents[3][3][2]);
    game.WriteStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 3; ++i)
        for (size_t j = 0; j != 3; ++j)
            if (i == 0 && j == 0)
                ASSERT_EQ(tensor[i][j][1], 1.);
            else
                ASSERT_EQ(tensor[i][j][1], 0.);
}

TEST(GameMap, Instantiation) {
    TicTacToe game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
}

TEST(GameMap, GetNotInDB) {
    TicTacToe game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
    size_t index;
    ASSERT_FALSE(game_map->Get(game, index));
}

TEST(GameMap, GetInDB) {
    TicTacToe game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
    game_map->Insert(game, 1ll);

    size_t index = 0;
    ASSERT_TRUE(game_map->Get(game, index));
    ASSERT_EQ(index, 1);
}

TEST(GameMap, InsertAlreadyInDB) {
    TicTacToe game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());

    ASSERT_EQ(game_map->GetSize(), 0);
    game_map->Insert(game, 1ll);
    ASSERT_EQ(game_map->GetSize(), 1);
    game_map->Insert(game, 1ll);
    ASSERT_EQ(game_map->GetSize(), 1);
}
