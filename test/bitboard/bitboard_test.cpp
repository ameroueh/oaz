#include "oaz/bitboard/bitboard.hpp"

#include <algorithm>
#include <iostream>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::bitboard;

TEST(InstantiationTest, Default) { BitBoard<4, 4> board; }

TEST(Set, Default) {
  BitBoard<4, 4> board;
  ASSERT_FALSE(board.Get(0, 0));
  board.Set(0, 0);
  ASSERT_TRUE(board.Get(0, 0));
}

TEST(Set, Test23) {
  BitBoard<4, 4> board;
  ASSERT_FALSE(board.Get(2, 3));
  board.Set(2, 3);
  ASSERT_TRUE(board.Get(2, 3));
}

TEST(Unset, Default) {
  BitBoard<4, 4> board;
  ASSERT_FALSE(board.Get(0, 0));
  board.Set(0, 0);
  ASSERT_TRUE(board.Get(0, 0));
  board.Unset(0, 0);
  ASSERT_FALSE(board.Get(0, 0));
}

TEST(Unset, Test23) {
  BitBoard<4, 4> board;
  ASSERT_FALSE(board.Get(2, 3));
  board.Set(2, 3);
  ASSERT_TRUE(board.Get(2, 3));
  board.Unset(2, 3);
  ASSERT_FALSE(board.Get(2, 3));
}

TEST(ColumnSum, Default) {
  BitBoard<4, 4> board;
  for (int i = 0; i != 4; ++i) board.Set(i, 1);
  ASSERT_EQ(board.ColumnSum(1), 4);
}

TEST(RowSum, Default) {
  BitBoard<4, 4> board;
  for (int i = 0; i != 4; ++i) board.Set(1, i);
  ASSERT_EQ(board.RowSum(1), 4);
}

TEST(Sum, Default) {
  BitBoard<4, 4> board;
  for (int i = 0; i != 4; ++i) board.Set(1, i);
  for (int i = 0; i != 4; ++i) board.Set(i, 2);
  ASSERT_EQ(board.Sum(), 7);
}

TEST(PositiveRowShift, Default) {
  BitBoard<6, 7> board{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
  BitBoard<6, 7> board2{{1, 0}, {2, 1}, {3, 2}, {4, 3}, {5, 4}};
  board.PositiveRowShift(1);
  ASSERT_EQ(board, board2);
}

TEST(NegativeRowShift, Default) {
  BitBoard<6, 7> board{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
  BitBoard<6, 7> board2{{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
  board.NegativeRowShift(1);
  ASSERT_EQ(board, board2);
}

TEST(PositiveColumnShift, Test1) {
  BitBoard<6, 7> board{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
  BitBoard<6, 7> board2{{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}};
  board.PositiveColumnShift(1);
  ASSERT_EQ(board, board2);
}

TEST(NegativeColumnShift, Default) {
  BitBoard<6, 7> board{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
  BitBoard<6, 7> board2{{1, 0}, {2, 1}, {3, 2}, {4, 3}, {5, 4}};
  board.NegativeColumnShift(1);
  ASSERT_EQ(board, board2);
}

TEST(NegativeColumnShift, Test2) {
  BitBoard<6, 7> board{{2, 0}, {3, 1}, {4, 2}, {5, 3}};
  BitBoard<6, 7> board2{{3, 0}, {4, 1}, {5, 2}};
  board.NegativeColumnShift(1);
  ASSERT_EQ(board, board2);
}

TEST(WriteToArray, Default) {
  BitBoard<6, 7> board;
  oaz::array::Array<6, 7> array;
  for (size_t i = 0; i != 4; ++i)
    for (size_t j = 0; j != 4; ++j) board.Set(i, j);
  board.WriteToArray(array);
  for (size_t i = 0; i != 4; ++i)
    for (size_t j = 0; j != 4; ++j) ASSERT_EQ(array[i][j], 1.);
}

TEST(ConstexprInstantiation, Default) {
  constexpr BitBoard<6, 7> board{{0, 0}, {0, 1}};
  ASSERT_TRUE(board.Get(0, 0));
  ASSERT_TRUE(board.Get(0, 1));
  ASSERT_FALSE(board.Get(1, 1));
}

TEST(IsContained, Default) {
  BitBoard<6, 7> board{{0, 0}, {0, 1}};
  BitBoard<6, 7> board2{{0, 0}};
  BitBoard<6, 7> board3{{0, 0}, {1, 0}};
  ASSERT_TRUE(board2.IsContainedIn(board));
  ASSERT_FALSE(board3.IsContainedIn(board));
}

TEST(LexicographicComponentLength, RowTest1) {
  BitBoard<6, 7> board{{0, 2}, {0, 3}, {0, 4}};
  BitBoard<6, 7> mask_board{{0, 0}, {0, 1}, {0, 2}, {0, 3},
                            {0, 4}, {0, 5}, {0, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 0, 3), 3);
}

TEST(LexicographicComponentLength, RowTest2) {
  BitBoard<6, 7> board{{0, 3}};
  BitBoard<6, 7> mask_board{{0, 0}, {0, 1}, {0, 2}, {0, 3},
                            {0, 4}, {0, 5}, {0, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 0, 3), 1);
}

TEST(LexicographicComponentLength, RowTest3) {
  BitBoard<6, 7> board;
  BitBoard<6, 7> mask_board{{0, 0}, {0, 1}, {0, 2}, {0, 3},
                            {0, 4}, {0, 5}, {0, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 0, 3), 0);
}

TEST(LexicographicComponentLength, ColumnTest1) {
  BitBoard<6, 7> board{{2, 0}, {3, 0}, {4, 0}};
  BitBoard<6, 7> mask_board{{0, 0}, {1, 0}, {2, 0}, {3, 0},
                            {4, 0}, {5, 0}, {6, 0}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 0), 3);
}

TEST(LexicographicComponentLength, ColumnTest2) {
  BitBoard<6, 7> board{{3, 0}};
  BitBoard<6, 7> mask_board{{0, 0}, {1, 0}, {2, 0}, {3, 0},
                            {4, 0}, {5, 0}, {6, 0}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 0), 1);
}

TEST(LexicographicComponentLength, ColumnTest3) {
  BitBoard<6, 7> board;
  BitBoard<6, 7> mask_board{{0, 0}, {1, 1}, {2, 2}, {3, 3},
                            {4, 4}, {5, 5}, {6, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 0), 0);
}

TEST(LexicographicComponentLength, DiagonalTest1) {
  BitBoard<6, 7> board{{2, 2}, {3, 3}, {4, 4}};
  BitBoard<6, 7> mask_board{{0, 0}, {1, 1}, {2, 2}, {3, 3},
                            {4, 4}, {5, 5}, {6, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 3), 3);
}

TEST(LexicographicComponentLength, DiagonalTest2) {
  BitBoard<6, 7> board{{3, 3}};
  BitBoard<6, 7> mask_board{{0, 0}, {1, 1}, {2, 2}, {3, 3},
                            {4, 4}, {5, 5}, {6, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 3), 1);
}

TEST(LexicographicComponentLength, DiagonalTest3) {
  BitBoard<6, 7> board;
  BitBoard<6, 7> mask_board{{0, 0}, {1, 1}, {2, 2}, {3, 3},
                            {4, 4}, {5, 5}, {6, 6}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 3), 0);
}

TEST(LexicographicComponentLength, DiagonalTest4) {
  BitBoard<6, 7> board{{0, 1}, {0, 2}, {3, 2}, {2, 3}, {1, 4}, {0, 5}};
  BitBoard<6, 7> mask_board{{5, 0}, {4, 1}, {3, 2}, {2, 3}, {1, 4}, {0, 5}};
  ASSERT_EQ(board.LexicographicComponentLength(mask_board, 3, 2), 4);
}
