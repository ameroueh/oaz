#ifndef __SELECTION_HPP__
#define __SELECTION_HPP__
#include <cmath>
#include <random>

#include "oaz/mcts/search_node.hpp"

static constexpr float C_EXPLORATION = 1.4142;

namespace oaz::mcts {

class Selector {
 public:
  virtual size_t operator()(SearchNode*) const = 0;
  virtual std::unique_ptr<Selector> Clone() const = 0;

  virtual ~Selector(){};
};

class UCTSelector : public Selector {
 public:
  size_t operator()(SearchNode* node) const {
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
  std::unique_ptr<Selector> Clone() const {
    return std::make_unique<UCTSelector>(*this);
  }

 private:
  float GetChildScore(SearchNode* parent, SearchNode* child) const {
    float q = (child->GetNVisits() == 0)
                  ? 0
                  : child->GetAccumulatedValue() / child->GetNVisits();
    float exploration_score =
        C_EXPLORATION *
        std::sqrt(std::log(parent->GetNVisits()) / (child->GetNVisits() + 1));
    return q + exploration_score;
  }
};

class AZSelector : public Selector {
 public:
  size_t operator()(SearchNode* node) const {
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
  std::unique_ptr<Selector> Clone() const {
    return std::make_unique<AZSelector>(*this);
  }

 private:
  float GetChildScore(SearchNode* parent, SearchNode* child) const {
    float q = (child->GetNVisits() == 0)
                  ? 0
                  : child->GetAccumulatedValue() / child->GetNVisits();
    float policy_score = C_EXPLORATION * child->GetPrior() *
                         std::sqrt(parent->GetNVisits()) /
                         (child->GetNVisits() + 1);
    return q + policy_score;
  }
};
}  // namespace oaz::mcts
#endif
