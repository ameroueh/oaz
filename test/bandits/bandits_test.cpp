#include "oaz/games/bandits.hpp"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::games;
using namespace testing;

TEST(InstantiationTest, Default) {
    Bandits game;
}

TEST(InstantiationTest, AvailableMoves) {
    Bandits game;
    std::vector<size_t> available_moves;
    game.GetAvailableMoves(available_moves);
    ASSERT_THAT(available_moves, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(PlayTest, Victory) {
    Bandits game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("3");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(-1., game.GetScore());
}

TEST(PlayTest, Victory2) {
    Bandits game;
    ASSERT_FALSE(game.IsFinished());
    game.PlayFromString("2");
    ASSERT_TRUE(game.IsFinished());
    ASSERT_EQ(1., game.GetScore());
}

TEST(Clone, Default) {
    Bandits game;
    game.PlayFromString("6");
    std::unique_ptr<Game> clone = game.Clone();
    Bandits* clone_ptr = dynamic_cast<Bandits*>(clone.get());
    ASSERT_TRUE(game == *clone_ptr);
}

TEST(GetCurrentPlayer, Default) {
    Bandits game;
    ASSERT_EQ(game.GetCurrentPlayer(), 0);
    game.PlayMove(0);
    ASSERT_EQ(game.GetCurrentPlayer(), 1);
}

TEST(ClassMethods, Default) {
    Bandits game;
    Game* game_ptr = &game;
    ASSERT_EQ(game_ptr->ClassMethods().GetMaxNumberOfMoves(), 10);
    ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[0], 10);
}

TEST(WriteStateToTensorMemory, Default) {
    Bandits game;
    game.PlayMove(0);
    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 10; ++i)
        if (i == 0)
            ASSERT_EQ(tensor[i], 1.);
        else
            ASSERT_EQ(tensor[i], 0.);
}

TEST(WriteCanonicalStateToTensorMemory, Default) {
    Bandits game;
    game.PlayMove(0);
    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteCanonicalStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 10; ++i)
        if (i == 0)
            ASSERT_EQ(tensor[i], 1.);
        else
            ASSERT_EQ(tensor[i], 0.);
}

TEST(InitialiseFromState, Default) {
    Bandits game;
    boost::multi_array<float, 1> test_board(boost::extents[10]);
    test_board[0] = 1.0f;
    game.InitialiseFromState(test_board.origin());

    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 10; ++i) {
        if (i == 0)
            ASSERT_EQ(tensor[i], 1.0f);
        else
            ASSERT_EQ(tensor[i], 0.);
    }
}

TEST(InitialiseFromState, CheckBoardCopy) {
    Bandits game;
    game.PlayFromString("0123");

    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteStateToTensorMemory(tensor.origin());

    Bandits game2;
    game2.InitialiseFromState(tensor.origin());
    ASSERT_TRUE(game == game2);
}

TEST(InitialiseFromCanonicalState, Default) {
    Bandits game;
    boost::multi_array<float, 1> test_board(boost::extents[10]);
    test_board[0] = 1.0;
    test_board[1] = 1.0;
    test_board[2] = 1.0;
    game.InitialiseFromCanonicalState(test_board.origin());

    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteCanonicalStateToTensorMemory(tensor.origin());
    for (size_t i = 0; i != 3; ++i)
        if (i == 0)
            ASSERT_EQ(tensor[i], 1.0f);
        else if (i == 1)
            ASSERT_EQ(tensor[i], 1.0f);
        else if (i == 2)
            ASSERT_EQ(tensor[i], 1.0f);
        else {
            ASSERT_EQ(tensor[i], 0.0f);
            ASSERT_EQ(tensor[i], 0.0f);
        }
}

TEST(InitialiseFromCanonicalState, CheckBoardCopy) {
    Bandits game;
    game.PlayFromString("0123");

    boost::multi_array<float, 1> tensor(boost::extents[10]);
    game.WriteCanonicalStateToTensorMemory(tensor.origin());

    Bandits game2;
    game2.InitialiseFromCanonicalState(tensor.origin());

    ASSERT_TRUE(game == game2);
}

TEST(GameMap, Instantiation) {
    Bandits game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
}

TEST(GameMap, GetNotInDB) {
    Bandits game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
    size_t index;
    ASSERT_FALSE(game_map->Get(game, index));
}

TEST(GameMap, GetInDB) {
    Bandits game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());
    game_map->Insert(game, 1ll);

    size_t index = 0;
    ASSERT_TRUE(game_map->Get(game, index));
    ASSERT_EQ(index, 1);
}

TEST(GameMap, InsertAlreadyInDB) {
    Bandits game;
    std::unique_ptr<oaz::games::Game::GameMap> game_map(
        game.ClassMethods().CreateGameMap());

    ASSERT_EQ(game_map->GetSize(), 0);
    game_map->Insert(game, 1ll);
    ASSERT_EQ(game_map->GetSize(), 1);
    game_map->Insert(game, 1ll);
    ASSERT_EQ(game_map->GetSize(), 1);
}
