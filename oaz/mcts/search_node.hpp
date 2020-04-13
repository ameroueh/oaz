#ifndef __SEARCH_NODE_HPP__
#define __SEARCH_NODE_HPP__

#include <memory>
#include <vector>

#include "stdint.h"

#include "oaz/mutex/mutex.hpp"

namespace oaz::mcts {
	template <class Move>
	class SearchNode {
		public:
			using tsize_t = uint32_t;
			using uniqueptr_t = std::unique_ptr<SearchNode>;
			using ptr_t = SearchNode*;
			using value_t = float;

			SearchNode():
				m_parent(nullptr),
				m_n_visits(0),
				m_acc_value(0.),
				m_prior(0.),
				m_is_blocked_for_evaluation(false)
			{}
			SearchNode(Move move, ptr_t parent, float prior):
				m_move(move),
				m_parent(parent),
				m_n_visits(0),
				m_acc_value(0.),
				m_prior(prior),
				m_is_blocked_for_evaluation(false)
			{}
			
			Move getMove() const {return m_move;}
			bool isRoot() const {return !m_parent;}
			bool isLeaf() const {return m_children.size() == 0;}
			void addChild(Move move, float prior) {
				uniqueptr_t child(new SearchNode<Move>(move, this, prior));
				m_children.push_back(std::move(child));
			}
			ptr_t getChild(tsize_t index) {
				return m_children[index].get();
			}
			tsize_t getNChildren() const {
				return m_children.size();
			}
			tsize_t getNVisits() const {
				return m_n_visits;
			}
			value_t getAccumulatedValue() const {
				return m_acc_value;
			}

			void incrementNVisits() {
				++m_n_visits;
			}

			void lock() {
				m_mutex.lock();
			}

			void unlock() {
				m_mutex.unlock();
			}

			bool IsBlockedForEvaluation() const {
				return m_is_blocked_for_evaluation;
			}

			void blockForEvaluation() {
				m_is_blocked_for_evaluation = true;
			}

			void unblockForEvaluation() {
				m_is_blocked_for_evaluation = false;
			}

			void addValue(float value) {
				m_acc_value += value;
			}	

			ptr_t getParent() {
				return m_parent;
			}

			float getPrior() const {
				return m_prior;
			}

		private:
			std::vector<uniqueptr_t> m_children;
			ptr_t m_parent;
			Move m_move;
			tsize_t m_n_visits;
			value_t m_acc_value;
			float m_prior;
			bool m_is_blocked_for_evaluation;
			oaz::mutex::SpinlockMutex m_mutex;
	};
}
#endif
