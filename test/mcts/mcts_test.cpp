#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/mcts.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::mcts;
using namespace oaz::games;
using namespace testing;


TEST (InstantiationTest, Default) {
	 SearchNode<ConnectFour::move_t>();
}

TEST (IsRootTest, Root) {
	 SearchNode<ConnectFour::move_t> node;
	 ASSERT_TRUE(node.isRoot());
}

TEST (IsRootTest, Child) {
	 SearchNode<ConnectFour::move_t> node;
	 SearchNode<ConnectFour::move_t> node2(0, &node);
	 
	 ASSERT_FALSE(node2.isRoot());
}

TEST (GetMoveTest, Child) {
	 SearchNode<ConnectFour::move_t> node;
	 SearchNode<ConnectFour::move_t> node2(0, &node);
	 
	 ASSERT_EQ(0, node2.getMove());
}

TEST (AddChild, Default) {
	 SearchNode<ConnectFour::move_t> node;
	 node.addChild(0);

	 ASSERT_EQ(1, node.getNChildren());
	 ASSERT_EQ(0, node.getChild(0)->getMove());
}
