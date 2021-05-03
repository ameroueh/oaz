#ifndef OAZ_UTILS_UTILS_HPP_
#define OAZ_UTILS_UTILS_HPP_

#include <iostream>
#include <vector>

#include "nlohmann/json.hpp"
#include "oaz/mcts/search_node.hpp"

template <class Game>
void loadBoardFromJson(const nlohmann::json& data,
                       typename Game::Board& board) {
  auto dimensions = Game::Board::Dimensions();
  for (int i = 0; i != dimensions[0]; ++i) {
    for (int j = 0; j != dimensions[1]; ++j) {
      for (int k = 0; k != dimensions[2]; ++k) {board[i][j][k] = data[i][j][k];}
    }
  }
}

template <class ArrayRef, int N>
struct RecursiveLoader {
  static void loadMultiArrayFromJson(const nlohmann::json& data,
                                     ArrayRef array) {
    for (int i = 0; i != array.shape()[0]; ++i) {
      RecursiveLoader<typename ArrayRef::template subarray<N - 1>::type,
                      N - 1>::loadMultiArrayFromJson(data[i], array[i]);
    }
  }
};

template <class ArrayRef>
struct RecursiveLoader<ArrayRef, 1> {
  static void loadMultiArrayFromJson(const nlohmann::json& data,
                                     ArrayRef array) {
    for (int i = 0; i != array.shape()[0]; ++i) {
      array[i] = data[i];
    }
  }
};

template <class Array>
void loadMultiArrayFromJson(const nlohmann::json& data, Array& array) {
  RecursiveLoader<Array, Array::dimensionality>::loadMultiArrayFromJson(data,
                                                                        array);
}

template <class Array>
void loadArrayFromJson(const nlohmann::json& data, Array& array) {
  for (size_t i = 0; i != array.size(); ++i) {array[i] = data[i];}
}

bool CheckSearchTree(oaz::mcts::SearchNode* node) {
  size_t n_visits = node->GetNVisits();
  bool overall_correct = true;

  if (!node->IsLeaf()) {
    bool correct_children = true;
    size_t n_children_visits = 0;

    for (size_t i = 0; i != node->GetNChildren(); ++i) {
      oaz::mcts::SearchNode* child = node->GetChild(i);
      n_children_visits += child->GetNVisits();
      correct_children &= CheckSearchTree(child);
    }

    if (!correct_children) {
      std::cout << "Incorrect children at " << node << std::endl;
    }
    bool correct = (n_visits == (n_children_visits + 1));
    if (!correct) {
      std::cout << "Incorrect node at " << node << "; n_children_visits "
                << n_children_visits << "; n_visits " << n_visits << std::endl;
    }

    overall_correct = correct && correct_children;
  }

  return overall_correct;
}
#endif  // OAZ_UTILS_UTILS_HPP_
