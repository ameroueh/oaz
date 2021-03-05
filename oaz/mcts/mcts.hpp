#ifndef __MCTS_HPP__
#define __MCTS_HPP__

#include <memory>
#include <vector>

#include "stdint.h"

namespace oaz::mcts {
template <class Move>
class SearchNode {
 public:
  typedef uint64_t tsize_t;
  typedef std::unique_ptr<SearchNode> uniqueptr_t;
  typedef SearchNode* ptr_t;

  SearchNode() : m_parent(nullptr) {}
  SearchNode(Move move, ptr_t parent) : m_move(move), m_parent(parent) {}

  Move getMove() const { return m_move; }
  bool isRoot() const { return !m_parent; }
  void addChild(Move move) {
    uniqueptr_t child(new SearchNode<Move>(move, this));
    m_children.push_back(std::move(child));
  }
  ptr_t getChild(tsize_t index) { return m_children[index].get(); }
  tsize_t getNChildren() const { return m_children.size(); }

 private:
  std::vector<uniqueptr_t> m_children;
  ptr_t m_parent;
  Move m_move;
};
}  // namespace oaz::mcts
#endif
