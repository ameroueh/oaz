#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"

#include <algorithm>
#include <string>

#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"


#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::games;
using namespace testing;
using namespace tensorflow;

using Board = tensorflow::TTypes<float, 3>::Tensor;

void playFromString(ConnectFour<Board>* game, std::string sMoves) {
	for(char& c : sMoves)
		game->playMove(c - '0');
}

TEST (InstantiationTest, Default) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
}

TEST (InstantiationTest, AvailableMoves) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_THAT(*(game.availableMoves()), ElementsAre(0, 1, 2, 3, 4, 5, 6));
}


TEST (ResetTest, Default) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	game.reset();
}


TEST (PlayTest, DoUndo) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	Tensor board2_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board2 = board2_tf.tensor<float, 3>();
	ConnectFour<Board> game(board);
	ConnectFour<Board> game2(board2);

	auto available_moves = *(game.availableMoves());
	
	for(int i=0; i!=available_moves.size(); ++i) {
		auto move_to_play = available_moves[i];
		game2.playMove(move_to_play);
		game2.undoMove(move_to_play);
		ASSERT_TRUE(game == game2);
	}
}

TEST (PlayTest, VerticalVictory) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "0103040");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, VerticalVictoryPlayer2) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "60103040");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, HorizontalVictory) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "051625364");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, HorizontalVictoryPlayer2) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "6051625364");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, FirstDiagonalVictory) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "12234334544");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, FirstDiagonalVictoryPlayer2) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "612234334544");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}


TEST (PlayTest, SecondDiagonalVictory) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "54432332122");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(1, game.score());
}

TEST (PlayTest, SecondDiagonalVictoryPlayer2) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_TRUE(~game.Finished());
	playFromString(&game, "654432332122");
	ASSERT_TRUE(game.Finished());
	ASSERT_EQ(-1, game.score());
}

TEST (PlayTest, TieTest) {
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
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
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	playFromString(
		&game, 
		"021302130213465640514455662233001144552636"
	);
	
	Tensor board2_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board2 = board2_tf.template tensor<float, 3>();
	
	ConnectFour<Board> game2(game, board2);
	ASSERT_TRUE(game == game2);
}

TEST (GetCurrentPlayer, Default) {
	
	Tensor board_tf(DT_FLOAT, TensorShape({7, 6, 2}));
	Board board = board_tf.template tensor<float, 3>();
	ConnectFour<Board> game(board);
	ASSERT_EQ(game.getCurrentPlayer(), 0);
	game.playMove(0);
	ASSERT_EQ(game.getCurrentPlayer(), 1);
}
