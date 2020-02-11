#ifndef __SELECTION_HPP__
#define __SELECTION_HPP__
#include <cmath>
#include <random>

static constexpr float C_EXPLORATION = 1.4142;

namespace oaz::mcts {
	template<class Node>
	float getChildScore(Node* parent, Node* child) {
		float q = (child->getNVisits() == 0) ? 0 : child->getAccumulatedValue() / child->getNVisits();
		float exploration_score = C_EXPLORATION * std::sqrt(std::log(parent->getNVisits()) / (child->getNVisits() + 1));
		return q + exploration_score;
	}

	template<class Node>
	typename Node::tsize_t getBestChildIndex(Node* node, std::mt19937& generator) {
		using tsize_t = typename Node::tsize_t;
		tsize_t best_child_index = 0;
		float best_score = 0;
		tsize_t n_best_scores = 0;
		std::uniform_real_distribution<> dis(0., 1.);
		for(tsize_t i=0; i!=node->getNChildren(); ++i) {
			float score = getChildScore<Node>(node, node->getChild(i)); 
			if(score > best_score) {
				best_score = score;
				best_child_index = i;
				n_best_scores = 1;
			} else if (score == best_score) {
				++n_best_scores;
				if (dis(generator) < (1. / ((float) n_best_scores))) {
					best_child_index = i;	
				}
			}
		}
		return best_child_index;
	}
}
#endif
