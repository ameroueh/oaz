#include <random>

#include "oaz/games/connect_four.hpp"

#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::mcts;
using namespace oaz::games;
using namespace testing;

using Game = ConnectFour;
using Node = SearchNode<Game::Move>;


TEST (InstantiationTest, Default) {
	 Node();
}

TEST (IsRootTest, Root) {
	Node node;
	ASSERT_TRUE(node.isRoot());
}

TEST (IsRootTest, Child) {
	 Node node;
	 Node node2(0, &node, 1.);
	 
	 ASSERT_FALSE(node2.isRoot());
}

TEST (GetMoveTest, Child) {
	 Node node;
	 Node node2(0, &node, 1.);
	 
	 ASSERT_EQ(0, node2.getMove());
}

TEST (GetAccumulatedValue, Default) {
	 Node node;
	 ASSERT_EQ(0., node.getAccumulatedValue());
}

TEST (GetNVisits, Default) {
	 Node node;
	 ASSERT_EQ(0, node.getNVisits());
}

TEST (AddChild, Default) {
	 Node node;
	 node.addChild(0, 1.);

	 ASSERT_EQ(1, node.getNChildren());
	 ASSERT_EQ(0, node.getChild(0)->getMove());
}


TEST (IsLeaf, Default) {
	 Node node;
	 ASSERT_TRUE(node.isLeaf());

	 node.addChild(0, 1.);
	 ASSERT_FALSE(node.isLeaf());
	 ASSERT_TRUE(node.getChild(0)->isLeaf());
}

TEST (LockUnlock, Default) {
	 Node node;
	 node.lock();
	 node.unlock();

}

TEST (BlockUnblockForEvaluation, Default) {
	Node node;
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

	 std::mt19937 generator;

	 Node node;
	 node.incrementNVisits();
	 node.addChild(0, 1.);
	 node.getChild(0)->incrementNVisits();
	 node.addChild(1, 1.);
	 node.getChild(1)->incrementNVisits();

	 UCTSelector<Node> selector;
	 ASSERT_EQ(0, selector(&node));

	 node.getChild(1)->addValue(1.);
	 ASSERT_EQ(1, selector(&node));
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
