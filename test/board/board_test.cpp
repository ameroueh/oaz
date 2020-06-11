#include "oaz/games/board.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST (InstantiationTest, Default) {
	ArrayBoard3D<float, 1, 2, 3> board;
}
