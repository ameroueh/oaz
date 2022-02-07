#include <iostream>
#include <random>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

using namespace oaz::mcts;
using namespace testing;

TEST(InstantiationTest, Default) { PriorSelector(); }

TEST(SamplingTest, Test1) {
  PriorSelector selector;
  SearchNode root;
  root.AddChild(0, 0, 0.0F);
  root.AddChild(0, 0, 1.0F);
  size_t index = selector(&root);
  ASSERT_EQ(index, 1);
}

TEST(SamplingTest, Test2) {
  PriorSelector selector;
  SearchNode root;
  root.AddChild(0, 0, 1.0F);
  root.AddChild(0, 0, 0.0F);
  size_t index = selector(&root);
  ASSERT_EQ(index, 0);
}

TEST(SamplingTest, LargeSampleEqualProbabilities) {
  PriorSelector selector;
  SearchNode root;
  root.AddChild(0, 0, 0.5F);
  root.AddChild(0, 0, 0.5F);

  size_t count0 = 0;
  size_t count1 = 0;
  for(size_t i=0; i!=1000; ++i) {
  	if(selector(&root) == 0) {
		++count0;
	} else {
		++count1;
	}
  }
  std::cout << "Counts: " << count0 << " " << count1 << std::endl;
}
