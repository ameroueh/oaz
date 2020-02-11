#include <iostream>
#include <vector>

#include "stdint.h"

#include "oaz/queue/queue.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

#include "oaz/mcts/search.hpp"
#include "oaz/mcts/backpropagation.hpp"

using namespace oaz::mcts;

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::selectNode(size_t index) {
	/* std::cout << index << " selected" << std::endl; */
	//Acquire resources
	nodeptr_t node = getNode(index);
	Game& game = m_game_evaluator->getGame(index);

	while(true) {
		node->lock();
		if(!node->isLeaf()) {
			
			/* std::cout << node << " not a leaf " << std::endl; */
			node->incrementNVisits();
			node->unlock();
			tsize_t child_index = getBestChildIndex<node_t>(node, m_generator);
			node = node->getChild(child_index);
			game.playMove(node->getMove());

		} else if (game.Finished()) {
			
			/* std::cout << node << " finished" << std::endl; */
			node->incrementNVisits();
			node->unlock();
			float score = game.score();
			float normalised_score = normaliseScore(score);
			/* std::cout << "Normalised score is " << normalised_score << std::endl; */
			node = backpropagateNode(node, game, normalised_score);
			m_reselection_q.lock();
			m_reselection_q.push(index);
			m_reselection_q.unlock();
			break;
		} else if (node->IsBlockedForEvaluation() ) {
			
			/* std::cout << node << " blocked for evaluation" << std::endl; */
			node->unlock();
			m_reselection_q.lock();
			m_reselection_q.push(index);
			m_reselection_q.unlock();
			break;
		} else {
			
			/* std::cout << node << " to evaluate" << std::endl; */
			node->incrementNVisits();
			node->blockForEvaluation();
			node->unlock();
			/* std::cout << "Hey index is " << index << std::endl; */
			/* std::cout << std::boolalpha << m_evaluation_q.empty() << std::endl; */
			m_evaluation_q.lock();
			m_evaluation_q.push(index);
			/* std::cout << std::boolalpha << m_evaluation_q.empty() << std::endl; */
			m_evaluation_q.unlock();
			break;
		}
	}

	setNode(index, node);
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::selectNodes() {
	while(true) {
		m_selection_q.lock();
		if(!m_selection_q.empty()) {
			size_t index = m_selection_q.front();
			m_selection_q.pop();
			m_selection_q.unlock();
			selectNode(index);
		} 
		else {
			m_selection_q.unlock();
			break;
		}
	}
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::evaluateNodes() {
	m_game_evaluator->evaluate();
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::expandNode(nodeptr_t node, Game& game, PolicyType& policy) {
	auto available_moves = game.availableMoves();
	for(auto move : *available_moves) {
		float prior = policy(move);
		node->addChild(move, prior);
	}
	node->unblockForEvaluation();
	/* std::cout << node << " expanded" << std::endl; */
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::expandAndBackpropagateNode(size_t index) {

	/* std::cout << "Hiya 2 index is " << index << std::endl; */
	float value = m_game_evaluator->getValue(index);
	float normalised_score = normaliseScore(value);
	/* std::cout << "Random value is " << value << std::endl; */
	PolicyType& policy = m_game_evaluator->getPolicy(index);
	/* std::cout << "Hiya 4" << std::endl; */
	nodeptr_t node = getNode(index);

	/* std::cout << node << " selected for expansion and backpropagation" << std::endl; */
	Game& game = m_game_evaluator->getGame(index);

	expandNode(node, game, policy);
	nodeptr_t new_node = backpropagateNode(node, game, normalised_score);

	setNode(index, new_node);
	
	/* std::cout << node << " expanded and backpropagated" << std::endl; */
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::expandAndBackpropagateNodes() {
	/* std::cout << "Expanding and backpropagating nodes" << std::endl; */
	while(true) {
		m_evaluation_q.lock();
		if(!m_evaluation_q.empty()) {
			/* std::cout << std::boolalpha << m_evaluation_q.empty() << std::endl; */
			size_t index = m_evaluation_q.front();
			m_evaluation_q.pop();
			m_evaluation_q.unlock();
			
			/* std::cout << "HiyA index is " << index << std::endl; */
			expandAndBackpropagateNode(index);

			m_reselection_q.lock();
			m_reselection_q.push(index);
			m_reselection_q.unlock();
		}
		else {
			m_evaluation_q.unlock();
			break;
		}
	}
}


template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::initialise(const Game& game) {
	m_nodes.resize(0);
	for(size_t i=0; i!=m_batch_size; ++i) {
		m_game_evaluator->getGame(i).set(game);
		m_nodes.push_back(&m_root);
		m_selection_q.push(i);
	}
}

template <class Game, class GameEvaluator>
Search<Game, GameEvaluator>::Search(tsize_t batch_size, const Game& game, GameEvaluator* evaluator):
	m_batch_size(batch_size),
	m_game_evaluator(evaluator) {
	initialise(game);
}

template <class Game, class GameEvaluator>
typename Search<Game, GameEvaluator>::nodeptr_t Search<Game, GameEvaluator>::backpropagateNode(
	nodeptr_t node, Game& game, float normalised_score) {
	while(!node->isRoot()) {
		float value = getValueFromScore(
			normalised_score, 
			game.getCurrentPlayer()
		);
		/* std::cout << "Value from score is " << value << std::endl; */
		node->lock();
		node->addValue(value);
		node->unlock();
		game.undoMove(node->getMove());
		node = node->getParent();
	}
	/* std::cout << node << " backpropagated" << std::endl; */
	return node;
}

template <class Game, class GameEvaluator>
size_t Search<Game, GameEvaluator>::getBatchSize() const {
	return m_batch_size;
}

template<class Game, class GameEvaluator>
typename Search<Game, GameEvaluator>::nodeptr_t Search<Game, GameEvaluator>::getNode(size_t index) {
	return m_nodes[index];
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::search(size_t n_iterations) {
	/* std::cout << "n_iterations " << n_iterations << std::endl; */
	for(size_t i=0; i!=n_iterations; ++i) {
		selectNodes();
		evaluateNodes();
		expandAndBackpropagateNodes();
		m_selection_q = m_reselection_q;
		oaz::queue::SafeQueue<size_t> empty;
		std::swap(m_reselection_q, empty);
	}
}

template <class Game, class GameEvaluator>
void Search<Game, GameEvaluator>::setNode(size_t index, nodeptr_t node) {
	m_nodes[index] = node;
}

template <class Game, class GameEvaluator>
typename Search<Game, GameEvaluator>::nodeptr_t Search<Game, GameEvaluator>::getTreeRoot() {
	return &m_root;
}


template <class Game, class GameEvaluator>
typename Search<Game, GameEvaluator>::move_t Search<Game, GameEvaluator>::getBestMove() {
	move_t best_move = 0;
	size_t best_n_visits = 0;
	for(size_t i=0; i!= m_root.getNChildren(); ++i) {
		nodeptr_t node = m_root.getChild(i);
		size_t node_n_visits = node->getNVisits();
		if(node_n_visits > best_n_visits) {
			best_n_visits = node_n_visits;
			best_move = node->getMove();
		}
	}
	return best_move;
}
