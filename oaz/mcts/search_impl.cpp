#include <algorithm>
#include <iostream>
#include <exception>
#include <vector>

#include "stdint.h"

#include "oaz/queue/queue.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

#include "oaz/mcts/search.hpp"
#include "oaz/mcts/backpropagation.hpp"

using namespace oaz::mcts;


SafeQueueNotifier::SafeQueueNotifier(oaz::queue::SafeQueue<size_t>* queue, size_t index):
	m_queue(queue),
	m_index(index) {
}

SafeQueueNotifier::SafeQueueNotifier():
	m_queue(nullptr),
	m_index(0) {
}

void SafeQueueNotifier::operator()() {
	m_queue->lock();
	m_queue->push(m_index);
	m_queue->unlock();
}


template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::enqueueForExpansionAndBackpropagation(size_t index) {
	m_expansion_and_backpropagation_q.lock();
	m_expansion_and_backpropagation_q.push(index);
	m_expansion_and_backpropagation_q.unlock();
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::selectNode(size_t index) {
	Node* node = getNode(index);
	Game& game = m_games[index];

	while(true) {
		node->lock();
		if(!node->isLeaf()) {
			
			node->incrementNVisits();
			node->unlock();
			size_t child_index = Selector()(node);
			node = node->getChild(child_index);
			game.playMove(node->getMove());
			setNode(index, node);

		/* } else if (game.Finished()) { */
			
		/* 	node->incrementNVisits(); */
		/* 	node->unlock(); */
		/* 	float score = game.score(); */
		/* 	float normalised_score = normaliseScore(score); */
		/* 	node = backpropagateNode(node, game, normalised_score); */
		/* 	setNode(index, node); */	
		/* 	incrementNCompletions(); */
		/* 	maybeSelect(index); */
		/* 	break; */

		} else if (node->IsBlockedForEvaluation() ) {
			
			pause(index);
			node->unlock();
			break;
		} else {
			node->incrementNVisits();
			node->blockForEvaluation();
			node->unlock();
			m_n_evaluation_requests++;
			m_evaluator->requestEvaluation(
				&game,
				&getValue(index),
				&getPolicy(index),
				SafeQueueNotifier(&m_expansion_and_backpropagation_q, index)
			);
			break;
		}
	}
}

template <class Game, class Evaluator, class Selector>
typename Game::Value& Search<Game, Evaluator, Selector>::getValue(size_t index) {
	return m_values[index];
}

template <class Game, class Evaluator, class Selector>
typename Game::Policy& Search<Game, Evaluator, Selector>::getPolicy(size_t index) {
	return m_policies[index];
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::pause(size_t index) {
	m_paused_nodes_lock.lock();
	m_paused_nodes[index] = getNode(index);
	++m_n_paused_nodes;
	m_paused_nodes_lock.unlock();
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::addDirichletNoise(Policy& policy) {
	if(m_noise_epsilon > 0.001) {
		Policy noise;
		float total_noise = 0.;
		for(size_t i=0; i!=noise.size(); ++i) {
			std::gamma_distribution<float> distribution(m_noise_alpha, 1.);
			noise[i] = distribution(m_generator);
			total_noise += noise[i];
		}
		for(size_t i=0; i!=policy.size(); ++i) {
			policy[i] = (1 - m_noise_epsilon) * policy[i] + m_noise_epsilon * noise[i] / total_noise;
		}
	}
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::expandNode(Node* node, Game& game, Policy& policy) {

	if(node->isRoot())
		addDirichletNoise(policy);

	auto available_moves = game.availableMoves();
	for(auto move : *available_moves) {
		float prior = policy[move];
		node->addChild(move, prior);
	}
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::unpause(Node* node) {
	m_paused_nodes_lock.lock();
	for(size_t index=0; index != getBatchSize(); ++index) {
		if(m_paused_nodes[index] == node) {
			m_selection_q.lock();
			m_selection_q.push(index);
			m_selection_q.unlock();
			--m_n_paused_nodes;
			m_paused_nodes[index] = nullptr;
		}
	}
	m_paused_nodes_lock.unlock();
}

template <class Game, class Evaluator, class Selector>
size_t Search<Game, Evaluator, Selector>::getNSelections() const {
	return m_n_selections;
}

template <class Game, class Evaluator, class Selector>
size_t Search<Game, Evaluator, Selector>::getNIterations() const {
	return m_n_iterations;
}


template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::maybeSelect(size_t index) {
	m_selection_lock.lock();
	if(getNSelections() < getNIterations()) {
		m_selection_q.lock();
		m_selection_q.push(index);
		m_selection_q.unlock();
		++m_n_selections;
	}
	m_selection_lock.unlock();
}

template <class Game, class Evaluator, class Selector>
Game& Search<Game, Evaluator, Selector>::getGame(size_t index) {
	return m_games[index];
}

/* template <class Game, class Evaluator, class Selector> */
/* typename Game::Value& Search<Game, Evaluator, Selector>::getValue(size_t index) { */
/* 	return m_values[index]; */
/* } */

/* template <class Game, class Evaluator, class Selector> */
/* typename Game::Policy& Search<Game, Evaluator, Selector>::getPolicy(size_t index) { */
/* 	return m_policies[index]; */
/* } */


template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::expandAndBackpropagateNode(size_t index) {
	
	float value = getValue(index);
	float normalised_score = normaliseScore(value);
	Policy& policy = getPolicy(index);
	Node* node = getNode(index);
	Game& game = getGame(index);

	/* std::cout << "WAIT" << std::endl; */
	node->lock();
	if (!game.Finished())
		expandNode(node, game, policy);
	unpause(node);
	node->unblockForEvaluation();
	node->unlock();
	/* std::cout << "DONE" << std::endl; */

	Node* new_node = backpropagateNode(node, game, normalised_score);

	setNode(index, new_node);
	incrementNCompletions();
	maybeSelect(index);
}

template <class Game, class Evaluator, class Selector>
bool Search<Game, Evaluator, Selector>::done() const {
	return getNCompletions() == getNIterations();
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::incrementNCompletions() {
	/* std::cout << "increment" << std::endl; */
	m_completion_lock.lock();
	++m_n_completions;
	m_completion_lock.unlock();
}

template <class Game, class Evaluator, class Selector>
size_t Search<Game, Evaluator, Selector>::getNCompletions() const {
	return m_n_completions;
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::initialise(const Game& game) {
	
	for(size_t index=0; index!=getBatchSize(); ++index) {
		m_games[index] = game;
		m_nodes[index] = &m_root;
		m_paused_nodes[index] = nullptr;
		maybeSelect(index);
	}
}

template <class Game, class Evaluator, class Selector>
Search<Game, Evaluator, Selector>::Search(
	const Game& game, SharedEvaluatorPointer evaluator, 
	size_t batch_size, 
	size_t n_iterations):
	m_batch_size(batch_size),
	m_n_iterations(n_iterations),
	m_n_selections(0),
	m_n_completions(0),
	m_n_paused_nodes(0),
	m_n_evaluation_requests(0),
	m_nodes(batch_size),
	m_paused_nodes(batch_size),
	m_games(batch_size),
	m_values(batch_size),
	m_policies(batch_size),
	m_evaluator(evaluator),
	m_noise_epsilon(0.),
	m_noise_alpha(1.) {
	initialise(game);
}

template <class Game, class Evaluator, class Selector>
Search<Game, Evaluator, Selector>::Search(
	const Game& game, SharedEvaluatorPointer evaluator, 
	size_t batch_size, 
	size_t n_iterations,
	float noise_epsilon,
	float noise_alpha):
	m_batch_size(batch_size),
	m_n_iterations(n_iterations),
	m_n_selections(0),
	m_n_completions(0),
	m_n_paused_nodes(0),
	m_n_evaluation_requests(0),
	m_nodes(batch_size),
	m_paused_nodes(batch_size),
	m_games(batch_size),
	m_values(batch_size),
	m_policies(batch_size),
	m_evaluator(evaluator),
	m_noise_epsilon(noise_epsilon),
	m_noise_alpha(noise_alpha) {
	initialise(game);
}

template <class Game, class Evaluator, class Selector>
typename Search<Game, Evaluator, Selector>::Node* Search<Game, Evaluator, Selector>::backpropagateNode(
	Node* node, Game& game, float normalised_score) {
	while(!node->isRoot()) {
		float value = getValueFromScore(
			normalised_score, 
			game.getCurrentPlayer()
		);
		node->lock();
		node->addValue(value);
		node->unlock();
		game.undoMove(node->getMove());
		node = node->getParent();
	}
	return node;
}

template <class Game, class Evaluator, class Selector>
size_t Search<Game, Evaluator, Selector>::getBatchSize() const {
	return m_batch_size;
}

template<class Game, class Evaluator, class Selector>
typename Search<Game, Evaluator, Selector>::Node* Search<Game, Evaluator, Selector>::getNode(size_t index) {
	return m_nodes[index];
}

template <class Game, class Evaluator, class Selector>
bool Search<Game, Evaluator, Selector>::work() {
	m_selection_q.lock();
	if(!m_selection_q.empty()) {
		size_t index = m_selection_q.front();
		m_selection_q.pop();
		m_selection_q.unlock();
		selectNode(index);
		return true;
	} else m_selection_q.unlock();
	
	m_expansion_and_backpropagation_q.lock();
	if(!m_expansion_and_backpropagation_q.empty()) {
		size_t index = m_expansion_and_backpropagation_q.front();
		m_expansion_and_backpropagation_q.pop();
		m_expansion_and_backpropagation_q.unlock();
		m_n_evaluation_requests--;
		expandAndBackpropagateNode(index);
		return true;
	} else m_expansion_and_backpropagation_q.unlock();

	return false;
}

template <class Game, class Evaluator, class Selector>
bool Search<Game, Evaluator, Selector>::waitingForEvaluation() {
	size_t effective_batch_size = std::min(getBatchSize(), getNIterations() - getNCompletions());
	size_t unfulfilled_requests = m_n_evaluation_requests - m_expansion_and_backpropagation_q.size();
	return unfulfilled_requests ==  (effective_batch_size - m_n_paused_nodes);
}


template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::setNode(size_t index, Node* node) {
	m_nodes[index] = node;
}

template <class Game, class Evaluator, class Selector>
typename Search<Game, Evaluator, Selector>::Node* Search<Game, Evaluator, Selector>::getTreeRoot() {
	return &m_root;
}


template <class Game, class Evaluator, class Selector>
typename Search<Game, Evaluator, Selector>::Move Search<Game, Evaluator, Selector>::getBestMove() {
	Move best_move = 0;
	size_t best_n_visits = 0;
	for(size_t i=0; i!= m_root.getNChildren(); ++i) {
		Node* node = m_root.getChild(i);
		size_t node_n_visits = node->getNVisits();
		if(node_n_visits > best_n_visits) {
			best_n_visits = node_n_visits;
			best_move = node->getMove();
		}
	}
	return best_move;
}

template <class Game, class Evaluator, class Selector>
void Search<Game, Evaluator, Selector>::getVisitCounts(Policy& move_counts) {
	for(size_t i=0; i!= m_root.getNChildren(); ++i) {
		Node* node = m_root.getChild(i);
		move_counts[node->getMove()] = node->getNVisits();
	}
}

template <class Game, class Evaluator, class Selector>
Search<Game, Evaluator, Selector>::~Search() {}
