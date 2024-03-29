#include "oaz/games/connect_four.hpp"

#include <algorithm>
#include <boost/multi_array.hpp>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::games;
using namespace testing;

TEST(InstantiationTest, Default) { ConnectFour game; }

TEST(GetAvailableMoves, Default) {
  ConnectFour game;
  std::vector<size_t> available_moves;
  game.GetAvailableMoves(&available_moves);
  ASSERT_THAT(available_moves, ElementsAre(0, 1, 2, 3, 4, 5, 6));
}

TEST(PlayTest, VerticalVictory) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("0103040");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, VerticalVictoryPlayer2) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("60103040");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, HorizontalVictory) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("051625364");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, HorizontalVictoryPlayer2) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("6051625364");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, FirstDiagonalVictory) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("12234334544");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, FirstDiagonalVictoryPlayer2) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("612234334544");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, SecondDiagonalVictory) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("54432332122");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(1, game.GetScore());
}

TEST(PlayTest, SecondDiagonalVictoryPlayer2) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("654432332122");
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(-1, game.GetScore());
}

TEST(PlayTest, TieTest) {
  ConnectFour game;
  ASSERT_FALSE(game.IsFinished());
  game.PlayFromString("021302130213465640514455662233001144552636");
  std::vector<size_t> available_moves;
  game.GetAvailableMoves(&available_moves);
  ASSERT_THAT(available_moves, ElementsAre());
  ASSERT_TRUE(game.IsFinished());
  ASSERT_EQ(0, game.GetScore());
}

TEST(Clone, Default) {
  ConnectFour game;
  game.PlayFromString("021302130213465640514455662233001144552636");
  std::unique_ptr<Game> clone = game.Clone();
  ConnectFour* clone_ptr = dynamic_cast<ConnectFour*>(clone.get());
  ASSERT_TRUE(game == *clone_ptr);
}

TEST(GetCurrentPlayer, Default) {
  ConnectFour game;
  ASSERT_EQ(game.GetCurrentPlayer(), 0);
  game.PlayMove(0);
  ASSERT_EQ(game.GetCurrentPlayer(), 1);
}

TEST(ClassMethods, Default) {
  ConnectFour game;
  Game* game_ptr = &game;
  ASSERT_EQ(game_ptr->ClassMethods().GetMaxNumberOfMoves(), 7);
  ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[0], 6);
  ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[1], 7);
  ASSERT_EQ(game_ptr->ClassMethods().GetBoardShape()[2], 2);
}

TEST(WriteStateToTensorMemory, Default) {
  ConnectFour game;
  game.PlayMove(0);
  game.PlayMove(5);
  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      if (i == 0 && j == 0)
        ASSERT_EQ(tensor[i][j][0], 1.);
      else
        ASSERT_EQ(tensor[i][j][0], 0.);
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      if (i == 0 && j == 5)
        ASSERT_EQ(tensor[i][j][1], 1.);
      else
        ASSERT_EQ(tensor[i][j][1], 0.);
}

TEST(WriteCanonicalStateToTensorMemory, Default) {
  ConnectFour game;
  game.PlayMove(0);
  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteCanonicalStateToTensorMemory(tensor.origin());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j) {
      if (i == 0 && j == 0)
        ASSERT_EQ(tensor[i][j][1], 1.);
      else
        ASSERT_EQ(tensor[i][j][1], 0.);
      ASSERT_EQ(tensor[i][j][0], 0.);
    }
}

TEST(InitialiseFromState, Default) {
  ConnectFour game;
  boost::multi_array<float, 3> test_board(boost::extents[6][7][2]);
  test_board[0][0][0] = 1.0;
  test_board[1][0][1] = 1.0;
  game.InitialiseFromState(test_board.origin());

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j) {
      if (i == 0 && j == 0)
        ASSERT_EQ(tensor[i][j][0], 1.0);
      else if (i == 1 && j == 0)
        ASSERT_EQ(tensor[i][j][1], 1.);
      else {
        ASSERT_EQ(tensor[i][j][0], 0.);
        ASSERT_EQ(tensor[i][j][1], 0.);
      }
    }
}

TEST(InitialiseFromState, CheckBoardCopy) {
  ConnectFour game;
  game.PlayFromString("0510055");

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());

  ConnectFour game2;
  game2.InitialiseFromState(tensor.origin());
  ASSERT_TRUE(game == game2);
}

TEST(InitialiseFromState, CheckBoardCopy2) {
  ConnectFour game;
  game.PlayFromString("0510055");

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());

  ConnectFour game2;
  game2.InitialiseFromState(tensor.origin());

  game.PlayMove(5);

  ASSERT_FALSE(game == game2);
}

TEST(InitialiseFromState, CheckGameInProgressInitialisation) {
  ConnectFour game;
  game.PlayFromString("0510055");

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());

  ConnectFour game2;
  game2.PlayFromString("0123313");
  game2.InitialiseFromState(tensor.origin());
  ASSERT_TRUE(game == game2);
}

TEST(InitialiseFromCanonicalState, Default) {
  ConnectFour game;
  boost::multi_array<float, 3> test_board(boost::extents[6][7][2]);
  test_board[0][0][0] = 1.0;
  test_board[1][0][1] = 1.0;
  test_board[2][0][0] = 1.0;
  game.InitialiseFromCanonicalState(test_board.origin());

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteCanonicalStateToTensorMemory(tensor.origin());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j) {
      if (i == 0 && j == 0)
        ASSERT_EQ(tensor[i][j][0], 1.0f);
      else if (i == 1 && j == 0)
        ASSERT_EQ(tensor[i][j][1], 1.0f);
      else if (i == 2 && j == 0)
        ASSERT_EQ(tensor[i][j][0], 1.0f);
      else {
        ASSERT_EQ(tensor[i][j][0], 0.0f);
        ASSERT_EQ(tensor[i][j][1], 0.0f);
      }
    }
}

TEST(InitialiseFromCanonicalState, CheckBoardCopy) {
  ConnectFour game;
  game.PlayFromString("0510055");

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteCanonicalStateToTensorMemory(tensor.origin());

  ConnectFour game2;
  game2.InitialiseFromCanonicalState(tensor.origin());

  ASSERT_TRUE(game == game2);
}

TEST(InitialiseFromCanonicalState, CheckBoardCopy2) {
  ConnectFour game;
  game.PlayFromString("0510055");

  boost::multi_array<float, 3> tensor(boost::extents[6][7][2]);
  game.WriteStateToTensorMemory(tensor.origin());

  ConnectFour game2;
  game2.InitialiseFromCanonicalState(tensor.origin());

  ASSERT_FALSE(game == game2);
}

TEST(GameMap, Instantiation) {
  ConnectFour game;
  std::unique_ptr<oaz::games::Game::GameMap> game_map(
      game.ClassMethods().CreateGameMap());
}

TEST(GameMap, GetNotInDB) {
  ConnectFour game;
  std::unique_ptr<oaz::games::Game::GameMap> game_map(
      game.ClassMethods().CreateGameMap());
  size_t index;
  ASSERT_FALSE(game_map->Get(game, &index));
}

TEST(GameMap, GetInDB) {
  ConnectFour game;
  std::unique_ptr<oaz::games::Game::GameMap> game_map(
      game.ClassMethods().CreateGameMap());
  game_map->Insert(game, 1ll);

  size_t index = 0;
  ASSERT_TRUE(game_map->Get(game, &index));
  ASSERT_EQ(index, 1);
}

TEST(GameMap, InsertAlreadyInDB) {
  ConnectFour game;
  std::unique_ptr<oaz::games::Game::GameMap> game_map(
      game.ClassMethods().CreateGameMap());

  ASSERT_EQ(game_map->GetSize(), 0);
  game_map->Insert(game, 1ll);
  ASSERT_EQ(game_map->GetSize(), 1);
  game_map->Insert(game, 1ll);
  ASSERT_EQ(game_map->GetSize(), 1);
}
