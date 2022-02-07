#ifndef OAZ_MCTS_SEARCH_NODE_HPP_
#define OAZ_MCTS_SEARCH_NODE_HPP_

#include <stdint.h>

#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "oaz/mutex/mutex.hpp"

namespace oaz::mcts {
class SearchNode {
 public:
  SearchNode()
      : m_parent(nullptr),
        m_player(0),
        m_n_visits(0),
        m_acc_value(0.),
        m_prior(0.),
        m_move(0),
        m_is_blocked_for_evaluation(false) {}
  SearchNode(size_t move, size_t player, SearchNode* parent, float prior)
      : m_move(move),
        m_player(player),
        m_parent(parent),
        m_n_visits(0),
        m_acc_value(0.),
        m_prior(prior),
        m_is_blocked_for_evaluation(false) {}
  SearchNode(const SearchNode& rhs)
      : m_move(rhs.m_move),
        m_player(rhs.m_player),
        m_parent(nullptr),
        m_n_visits(rhs.m_n_visits),
        m_acc_value(rhs.m_acc_value),
        m_prior(rhs.m_prior),
        m_is_blocked_for_evaluation(false) {
    for (size_t i = 0; i != rhs.GetNChildren(); ++i) {
      std::unique_ptr<SearchNode> child =
          std::make_unique<SearchNode>(*rhs.m_children[i]);
      child->SetParent(this);
      m_children.push_back(std::move(child));
    }
  }

  SearchNode(SearchNode&& rhs) noexcept
      : m_move(rhs.m_move),
        m_player(rhs.m_player),
        m_parent(nullptr),
        m_n_visits(rhs.m_n_visits),
        m_acc_value(rhs.m_acc_value),
        m_prior(rhs.m_prior),
        m_is_blocked_for_evaluation(false) {
    m_children.resize(rhs.GetNChildren());
    for (size_t i = 0; i != rhs.GetNChildren(); ++i) {
      m_children[i] = std::move(rhs.m_children[i]);
      rhs.m_children[i] = std::move(rhs.m_children[i]);
    }
  }

  SearchNode& operator=(const SearchNode&) = delete;
  SearchNode& operator=(SearchNode&&) = delete;
  ~SearchNode() = default;

  size_t GetMove() const { return m_move; }
  size_t GetPlayer() const { return m_player; }
  bool IsRoot() const { return m_parent == nullptr; }
  bool IsLeaf() const { return m_children.empty(); }
  void AddChild(size_t move, size_t player, float prior) {
    std::unique_ptr<SearchNode> child(
        new SearchNode(move, player, this, prior));
    m_children.push_back(std::move(child));
  }
  SearchNode* GetChild(size_t index) { return m_children[index].get(); }
  size_t GetNChildren() const { return m_children.size(); }
  size_t GetNVisits() const { return m_n_visits; }
  float GetAccumulatedValue() const { return m_acc_value; }

  void IncrementNVisits() { ++m_n_visits; }

  void Lock() { m_mutex.Lock(); }

  void Unlock() { m_mutex.Unlock(); }

  bool IsBlockedForEvaluation() const { return m_is_blocked_for_evaluation; }

  void BlockForEvaluation() { m_is_blocked_for_evaluation = true; }

  void UnblockForEvaluation() { m_is_blocked_for_evaluation = false; }

  void AddValue(float value) { m_acc_value += value; }

  SearchNode* GetParent() { return m_parent; }

  const float& GetPrior() const { return m_prior; }

  class CPriorIterator : public std::iterator<std::input_iterator_tag, const float, void, const float*, const float&>
{
    public: 

      CPriorIterator(SearchNode* search_node, size_t child_index): m_search_node(search_node), m_child_index(child_index) {}

      reference operator*() const { return m_search_node->GetChild(m_child_index)->GetPrior(); }
      pointer operator->() const { return &m_search_node->GetChild(m_child_index)->GetPrior(); }
      CPriorIterator& operator++() { m_child_index++; return *this; }
      CPriorIterator operator++(int) { CPriorIterator tmp = *this; ++(*this); return tmp; }

      friend bool operator== (const CPriorIterator& a, const CPriorIterator& b) { return a.m_search_node == b.m_search_node && a.m_child_index == b.m_child_index; };
      friend bool operator!= (const CPriorIterator& a, const CPriorIterator& b) { return !(a == b); }

    private:
      SearchNode* m_search_node;
      size_t m_child_index;

  };

  CPriorIterator GetPriorCBegin() { return CPriorIterator(this, 0); }
  CPriorIterator GetPriorCEnd() { return CPriorIterator(this, GetNChildren()); }

 private:
  void SetParent(SearchNode* parent) { m_parent = parent; }

  std::vector<std::unique_ptr<SearchNode>> m_children;
  SearchNode* m_parent;
  size_t m_player;
  size_t m_move;
  size_t m_n_visits;
  float m_acc_value;
  float m_prior;
  bool m_is_blocked_for_evaluation;
  oaz::mutex::SpinlockMutex m_mutex;
};
}  // namespace oaz::mcts
#endif  // OAZ_MCTS_SEARCH_NODE_HPP_
