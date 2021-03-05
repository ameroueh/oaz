#ifndef __SEARCH_NODE_SERIALISATION_HPP__
#define __SEARCH_NODE_SERIALISATION_HPP__

#include <iostream>
#include <map>
#include <queue>
#include <string>

#include "nlohmann/json.hpp"
#include "oaz/mcts/search_node.hpp"

using json = nlohmann::json;

namespace oaz::mcts {

template <class Move>
std::string serialiseTreeToJson(SearchNode<Move>* root, size_t max_level = 1) {
  using NodePtr = SearchNode<Move>*;
  std::map<NodePtr, size_t> index_map;

  size_t index = 0;
  std::queue<NodePtr> bfs_queue;
  std::queue<NodePtr> next_bfs_queue;

  bfs_queue.push(root);
  index_map[root] = index;
  json serialised_nodes;

  size_t current_level = 0;
  while (!bfs_queue.empty()) {
    if (current_level > max_level) break;

    auto node = bfs_queue.front();
    bfs_queue.pop();

    size_t node_index = index_map[node];

    json serialised_node = serialiseNodeAsJson(node, node_index);

    for (size_t i = 0; i != node->getNChildren(); ++i) {
      ++index;
      serialised_node["children"].push_back(index);
      auto child = node->getChild(i);
      next_bfs_queue.push(child);
      index_map[child] = index;
    }

    serialised_nodes[node_index] = serialised_node;
    if (bfs_queue.empty()) {
      bfs_queue = next_bfs_queue;
      next_bfs_queue = std::queue<NodePtr>();
      ++current_level;
    }
  }
  return serialised_nodes.dump();
}

template <class Move>
json serialiseNodeAsJson(SearchNode<Move>* node, size_t node_index) {
  json serialised_node;
  serialised_node["n_visits"] = node->getNVisits();
  serialised_node["children"] = {};
  serialised_node["move"] = node->getMove();
  serialised_node["id"] = node_index;
  serialised_node["acc_value"] = node->getAccumulatedValue();
  serialised_node["prior"] = node->getPrior();
  return serialised_node;
}
}  // namespace oaz::mcts

#endif
