#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"

#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::mcts;
using namespace oaz::games;
using namespace testing;

using Board = ArrayBoard3D<float, 7, 6, 2>;
using Game = ConnectFour<Board>;
using Node = SearchNode<Game::move_t>;


TEST (InstantiationTest, Default) {
	 SearchNode<Game::move_t>();
}

TEST (IsRootTest, Root) {
	 SearchNode<Game::move_t> node;
	 ASSERT_TRUE(node.isRoot());
}

TEST (IsRootTest, Child) {
	 SearchNode<Game::move_t> node;
	 SearchNode<Game::move_t> node2(0, &node, 1.);
	 
	 ASSERT_FALSE(node2.isRoot());
}

TEST (GetMoveTest, Child) {
	 SearchNode<Game::move_t> node;
	 SearchNode<Game::move_t> node2(0, &node, 1.);
	 
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
	 node.addChild(0, 1.);

	 ASSERT_EQ(1, node.getNChildren());
	 ASSERT_EQ(0, node.getChild(0)->getMove());
}


TEST (IsLeaf, Default) {
	 SearchNode<Game::move_t> node;
	 ASSERT_TRUE(node.isLeaf());

	 node.addChild(0, 1.);
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

TEST (AddValue, Default) {
	Node node;
	ASSERT_EQ(0., node.getAccumulatedValue());

	node.addValue(1.);
	ASSERT_EQ(1., node.getAccumulatedValue());
}

TEST (Selection, Default) {
	 Node node;
	 node.incrementNVisits();
	 node.addChild(0, 1.);
	 node.getChild(0)->incrementNVisits();
	 node.addChild(1, 1.);
	 node.getChild(1)->incrementNVisits();
	 ASSERT_EQ(0, getBestChildIndex<Node>(&node));

	 node.getChild(1)->addValue(1.);
	 ASSERT_EQ(1, getBestChildIndex<Node>(&node));
}

TEST (GetParent, Default) {
	Node root;
	root.addChild(0, 1.);
	Node* child = root.getChild(0);
	ASSERT_EQ(&root, child->getParent());
}

TEST (GetPrior, Default) {
	Node root;
	root.addChild(0, 0.5);
	ASSERT_EQ(root.getChild(0)->getPrior(), 0.5);
}
