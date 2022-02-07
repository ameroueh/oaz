#include <random>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

using namespace oaz::mcts;
using namespace testing;

TEST(InstantiationTest, Default) { SearchNode(); }

TEST(IsRootTest, Root) {
  SearchNode node;
  ASSERT_TRUE(node.IsRoot());
}

TEST(IsRootTest, Child) {
  SearchNode node;
  SearchNode node2(0, 0, &node, 1.);

  ASSERT_FALSE(node2.IsRoot());
}

TEST(GetMoveTest, Child) {
  SearchNode node;
  SearchNode node2(0, 0, &node, 1.);

  ASSERT_EQ(0, node2.GetMove());
}

TEST(GetAccumulatedValue, Default) {
  SearchNode node;
  ASSERT_EQ(0., node.GetAccumulatedValue());
}

TEST(GetNVisits, Default) {
  SearchNode node;
  ASSERT_EQ(0, node.GetNVisits());
}

TEST(AddChild, Default) {
  SearchNode node;
  node.AddChild(0, 0, 1.);

  ASSERT_EQ(1, node.GetNChildren());
  ASSERT_EQ(0, node.GetChild(0)->GetMove());
}

TEST(IsLeaf, Default) {
  SearchNode node;
  ASSERT_TRUE(node.IsLeaf());

  node.AddChild(0, 0, 1.);
  ASSERT_FALSE(node.IsLeaf());
  ASSERT_TRUE(node.GetChild(0)->IsLeaf());
}

TEST(LockUnlock, Default) {
  SearchNode node;
  node.Lock();
  node.Unlock();
}

TEST(BlockUnblockForEvaluation, Default) {
  SearchNode node;
  ASSERT_FALSE(node.IsBlockedForEvaluation());
  node.BlockForEvaluation();
  ASSERT_TRUE(node.IsBlockedForEvaluation());
  node.UnblockForEvaluation();
  ASSERT_FALSE(node.IsBlockedForEvaluation());
}

TEST(AddValue, Default) {
  SearchNode node;
  ASSERT_EQ(0., node.GetAccumulatedValue());

  node.AddValue(1.);
  ASSERT_EQ(1., node.GetAccumulatedValue());
}

TEST(Selection, Default) {
  std::mt19937 generator;

  SearchNode node;
  node.IncrementNVisits();
  node.AddChild(0, 0, 1.);
  node.GetChild(0)->IncrementNVisits();
  node.AddChild(1, 0, 1.);
  node.GetChild(1)->IncrementNVisits();

  UCTSelector selector;
  ASSERT_EQ(0, selector(&node));

  node.GetChild(1)->AddValue(1.);
  ASSERT_EQ(1, selector(&node));
}

TEST(GetParent, Default) {
  SearchNode root;
  root.AddChild(0, 0, 1.);
  SearchNode* child = root.GetChild(0);
  ASSERT_EQ(&root, child->GetParent());
}

TEST(GetPrior, Default) {
  SearchNode root;
  root.AddChild(0, 0, 0.5);
  ASSERT_EQ(root.GetChild(0)->GetPrior(), 0.5);
}

TEST(PriorIterator, Basic) {
  SearchNode root;
  ASSERT_EQ(root.GetPriorCBegin(), root.GetPriorCEnd());
}

TEST(PriorIterator, OneChild) {
  SearchNode root;
  root.AddChild(0, 0, 0.5);
  ASSERT_EQ(*(root.GetPriorCBegin()), 0.5);
  ASSERT_NE(root.GetPriorCBegin(), root.GetPriorCEnd());
}

TEST(PriorIterator, TwoChildren) {
  SearchNode root;
  root.AddChild(0, 0, 0.5);
  root.AddChild(0, 0, 0.6);
  ASSERT_EQ(*(++root.GetPriorCBegin()), 0.6F);
}

