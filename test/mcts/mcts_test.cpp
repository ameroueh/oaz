#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"

#include "oaz/mcts/search_node.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::mcts;
using namespace oaz::games;
using namespace testing;

using Board = ArrayBoard3D<float, 7, 6, 2>;
using Game = ConnectFour<Board>;


TEST (InstantiationTest, Default) {
	 SearchNode<Game::move_t>();
}

TEST (IsRootTest, Root) {
	 SearchNode<Game::move_t> node;
	 ASSERT_TRUE(node.isRoot());
}

TEST (IsRootTest, Child) {
	 SearchNode<Game::move_t> node;
	 SearchNode<Game::move_t> node2(0, &node);
	 
	 ASSERT_FALSE(node2.isRoot());
}

TEST (GetMoveTest, Child) {
	 SearchNode<Game::move_t> node;
	 SearchNode<Game::move_t> node2(0, &node);
	 
	 ASSERT_EQ(0, node2.getMove());
}

TEST (GetAccumulatedValue, Default) {
	 SearchNode<Game::move_t> node;
	 ASSERT_EQ(0., node.getAccumulatedValue());
}

TEST (GetNVisits, Default) {
	 SearchNode<Game::move_t> node;
	 ASSERT_EQ(0, node.getNVisits());
}

TEST (AddChild, Default) {
	 SearchNode<Game::move_t> node;
	 node.addChild(0);

	 ASSERT_EQ(1, node.getNChildren());
	 ASSERT_EQ(0, node.getChild(0)->getMove());
}


TEST (IsLeaf, Default) {
	 SearchNode<Game::move_t> node;
	 ASSERT_TRUE(node.isLeaf());

	 node.addChild(0);
	 ASSERT_FALSE(node.isLeaf());
	 ASSERT_TRUE(node.getChild(0)->isLeaf());
}

TEST (LockUnlock, Default) {
	 SearchNode<Game::move_t> node;
	 node.lock();
	 node.unlock();

}

TEST (BlockUnblockForEvaluation, Default) {
	SearchNode<Game::move_t> node;
	ASSERT_FALSE(node.IsBlockedForEvaluation());
	node.blockForEvaluation();
	ASSERT_TRUE(node.IsBlockedForEvaluation());
	node.unblockForEvaluation();
	ASSERT_FALSE(node.IsBlockedForEvaluation());
}
