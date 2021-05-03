#ifndef OAZ_MCTS_SELECTION_HPP_
#define OAZ_MCTS_SELECTION_HPP_

#include <cmath>
#include <memory>
#include <random>

#include "oaz/mcts/search_node.hpp"

namespace oaz::mcts {

static constexpr float C_EXPLORATION = 1.4142;

class Selector {
 public:
  virtual size_t operator()(oaz::mcts::SearchNode*) const = 0;
  virtual std::unique_ptr<Selector> Clone() const = 0;

  virtual ~Selector() {}
  Selector() = default;
  Selector(const Selector&) = default;
  Selector(Selector&&) = default;
  Selector& operator=(const Selector&) = default;
  Selector& operator=(Selector&&) = default;
};

class UCTSelector : public Selector {
 public:
  size_t operator()(oaz::mcts::SearchNode* node) const override {
    size_t best_child_index = 0;
    float best_score = 0;
    for (size_t i = 0; i != node->GetNChildren(); ++i) {
      float score = GetChildScore(node, node->GetChild(i));
      if (score > best_score) {
        best_score = score;
        best_child_index = i;
      }
    }
    return best_child_index;
  }
  std::unique_ptr<Selector> Clone() const override {
    return std::make_unique<UCTSelector>(*this);
  }

 private:
  static float GetChildScore(oaz::mcts::SearchNode* parent,
                             oaz::mcts::SearchNode* child) {
    float q = (child->GetNVisits() == 0)
                  ? 0
                  : child->GetAccumulatedValue() / child->GetNVisits();
    float exploration_score =
        C_EXPLORATION *
        static_cast<float>(std::sqrt(std::log(parent->GetNVisits()))) /
        static_cast<float>((child->GetNVisits() + 1));
    return q + exploration_score;
  }
};

class AZSelector : public Selector {
 public:
  size_t operator()(oaz::mcts::SearchNode* node) const override {
    size_t best_child_index = 0;
    float best_score = 0;
    for (size_t i = 0; i != node->GetNChildren(); ++i) {
      float score = GetChildScore(node, node->GetChild(i));
      if (score > best_score) {
        best_score = score;
        best_child_index = i;
      }
    }
    return best_child_index;
  }
  std::unique_ptr<Selector> Clone() const override {
    return std::make_unique<AZSelector>(*this);
  }

 private:
  static float GetChildScore(oaz::mcts::SearchNode* parent,
                             oaz::mcts::SearchNode* child) {
    float q = (child->GetNVisits() == 0)
                  ? 0
                  : child->GetAccumulatedValue() / child->GetNVisits();
    float policy_score = C_EXPLORATION * child->GetPrior() *
                         static_cast<float>(std::sqrt(parent->GetNVisits())) /
                         static_cast<float>(child->GetNVisits() + 1);
    return q + policy_score;
  }
};
}  // namespace oaz::mcts
#endif  // OAZ_MCTS_SELECTION_HPP_
