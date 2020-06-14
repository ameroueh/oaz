#ifndef __SELECTION_HPP__
#define __SELECTION_HPP__
#include <cmath>
#include <random>

static constexpr float C_EXPLORATION = 1.4142;

namespace oaz::mcts {
	template<class Node>
	class UCTSelector {
		public:
			size_t operator()(Node* node) {
				size_t best_child_index = 0;
				float best_score = 0;
				for(size_t i=0; i!=node->getNChildren(); ++i) {
					float score = getChildScore(node, node->getChild(i)); 
					if(score > best_score) {
						best_score = score;
						best_child_index = i;
					} 
				}
				return best_child_index;
			}

		private:
			float getChildScore(Node* parent, Node* child) {
				float q = (child->getNVisits() == 0) ? 0 : child->getAccumulatedValue() / child->getNVisits();
				float exploration_score = C_EXPLORATION * std::sqrt(std::log(parent->getNVisits()) / (child->getNVisits() + 1));
				return q + exploration_score;
			}
	};

	template <class Node>
	class AZSelector {
		public:
			size_t operator()(Node* node) {
				size_t best_child_index = 0;
				float best_score = 0;
				for(size_t i=0; i!=node->getNChildren(); ++i) {
					float score = getChildScore(node, node->getChild(i)); 
					if(score > best_score) {
						best_score = score;
						best_child_index = i;
					} 
				}
				return best_child_index;
			}
		private:
			float getChildScore(Node* parent, Node* child) {
				float q = (child->getNVisits() == 0) ? 0 : child->getAccumulatedValue() / child->getNVisits();
				float policy_score = C_EXPLORATION * child->getPrior() * std::sqrt(parent->getNVisits()) / (child->getNVisits() + 1);
				return q + policy_score;
			}
	};
}
#endif
