#include <algorithm>
#include <iostream>
#include <exception>
#include <vector>
#include <random>

#include "stdint.h"

#include "oaz/queue/queue.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

#include "oaz/mcts/search.hpp"
#include "oaz/mcts/backpropagation.hpp"

#include "spdlog/spdlog.h"

using namespace oaz::mcts;


template <class Game, class Selector>
Search<Game, Selector>::SelectionTask::SelectionTask(
	Search<Game, Selector>* search,
	size_t index):
	m_index(index),
	m_search(search) {
		m_search->handleCreatedTask();
}

template <class Game, class Selector>
Search<Game, Selector>::SelectionTask::SelectionTask():
	m_index(0),
	m_search(nullptr) {}


template <class Game, class Selector>
void Search<Game, Selector>::SelectionTask::operator()() {
	spdlog::debug("Search {:p} index {} selection task", (void*)m_search, m_index);
	m_search->selectNode(m_index);
	m_search->handleFinishedTask();
}

template <class Game, class Selector>
Search<Game, Selector>::SelectionTask::~SelectionTask () {}

template <class Game, class Selector>
Search<Game, Selector>::ExpansionAndBackpropagationTask::ExpansionAndBackpropagationTask(
	Search<Game, Selector>* search,
	size_t index):
	m_index(index),
	m_search(search) {
		m_search->handleCreatedTask();
}

template <class Game, class Selector>
Search<Game, Selector>::ExpansionAndBackpropagationTask::ExpansionAndBackpropagationTask():
	m_index(0),
	m_search(nullptr) {}


template <class Game, class Selector>
void Search<Game, Selector>::ExpansionAndBackpropagationTask::operator()() {
	spdlog::debug("Search {:p} index {} expansion and backpropagation task", (void*)m_search, m_index);
	m_search->expandAndBackpropagateNode(m_index);
	m_search->handleFinishedTask();
}

template <class Game, class Selector>
Search<Game, Selector>::ExpansionAndBackpropagationTask::~ExpansionAndBackpropagationTask () {}

template <class Game, class Selector>
void Search<Game, Selector>::selectNode(size_t index) {
	
	spdlog::debug("Search {:p} index {} selectNode", (void*)this, index);
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

		} else if (node->IsBlockedForEvaluation() ) {
			
			spdlog::debug("Search {:p} index {} selectNode node {:p} blocked for evaluation, pausing", (void*)this, index, (void*)node);
			pause(index);
			node->unlock();
			break;
		} else {
			
			spdlog::debug("Search {:p} index {} selectNode node {:p} requesting evaluation", (void*)this, index, (void*)node);
			node->incrementNVisits();
			if (!game.Finished()) {
				spdlog::debug("Search {:p} index {} selectNode node {:p} game not finished, blocking evaluations", (void*)this, index, (void*)node);
				node->blockForEvaluation();
			}
			node->unlock();
			m_n_evaluation_requests++;
			m_expansion_and_backpropagation_tasks[index] = 
				ExpansionAndBackpropagationTask(this, index); 

			m_evaluator->requestEvaluation(
				&game,
				&getValue(index),
				&getPolicy(index),
				&m_expansion_and_backpropagation_tasks[index]
			);
			spdlog::debug("Search {:p} index {} selectNode node {:p} returning", (void*)this, index, (void*)node);
			break;
		}
	}
}

template <class Game, class Selector>
typename Game::Value& Search<Game, Selector>::getValue(size_t index) {
	return m_values[index];
}

template <class Game, class Selector>
void Search<Game, Selector>::seedRNG(size_t seed) {
	m_generator.seed(seed);	
}


template <class Game, class Selector>
typename Game::Policy& Search<Game, Selector>::getPolicy(size_t index) {
	return m_policies[index];
}

template <class Game, class Selector>
void Search<Game, Selector>::pause(size_t index) {
	m_paused_nodes[index] = getNode(index);
}

template <class Game, class Selector>
void Search<Game, Selector>::addDirichletNoise(Policy& policy) {
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

template <class Game, class Selector>
void Search<Game, Selector>::expandNode(Node* node, Game& game, Policy& policy) {
	spdlog::debug("Search {:p} node {:p} expandNode", (void*)this, (void*)node);

	if(node->isRoot())
		addDirichletNoise(policy);

	auto available_moves = game.availableMoves();

	for(auto move : *available_moves) {
		float prior = policy[move];
		node->addChild(move, prior);
	}
}

template <class Game, class Selector>
void Search<Game, Selector>::unpause(Node* node) {
	
	spdlog::debug("Search {:p} node {:p} unpause", (void*)this, (void*)node);
	for(size_t index=0; index != getBatchSize(); ++index) {
		if(m_paused_nodes[index] == node) {
			m_selection_tasks[index] = SelectionTask(this, index);
			m_thread_pool->enqueue(&m_selection_tasks[index]);
			m_paused_nodes[index] = nullptr;
			spdlog::debug("Search {:p} node {:p} unpause unpaused index {}", (void*)this, (void*)node, index);
		}
	}
}

template <class Game, class Selector>
size_t Search<Game, Selector>::getNSelections() const {
	return m_n_selections;
}

template <class Game, class Selector>
size_t Search<Game, Selector>::getNIterations() const {
	return m_n_iterations;
}


template <class Game, class Selector>
void Search<Game, Selector>::maybeSelect(size_t index) {

	spdlog::debug("Search {:p} index {} maybeSelect n_selections {} / {}; n_completions {} / {}", 
		(void*)this, 
		index,
		getNSelections(),
		getNIterations(),
		getNCompletions(),
		getNIterations()
	);

	m_selection_lock.lock();
	if(getNSelections() < getNIterations()) {
		spdlog::debug("Search {:p} index {} maybeSelect new selection", (void*)this, index);
		++m_n_selections;
		m_selection_lock.unlock();
		m_selection_tasks[index] = SelectionTask(this, index);
		m_thread_pool->enqueue(&m_selection_tasks[index]);
	} else m_selection_lock.unlock();
	spdlog::debug("Search {:p} index {} maybeSelect returning", (void*)this, index);
}

template <class Game, class Selector>
Game& Search<Game, Selector>::getGame(size_t index) {
	return m_games[index];
}

template <class Game, class Selector>
void Search<Game, Selector>::expandAndBackpropagateNode(size_t index) {
	spdlog::debug("Search {:p} index {} expandAndBackpropagate", (void*)this, index);
	
	float value = getValue(index);
	float normalised_score = normaliseScore(value);
	Policy& policy = getPolicy(index);
	Node* node = getNode(index);
	Game& game = getGame(index);

	if (!game.Finished()) {
		spdlog::debug("Search {:p} index {} expandAndBackpropagate game not finished", (void*)this, index);
		node->lock();
		expandNode(node, game, policy);
		node->unblockForEvaluation();
		unpause(node);
		node->unlock();
	}
	
	Node* new_node = backpropagateNode(node, game, normalised_score);

	setNode(index, new_node);
	incrementNCompletions();
	maybeSelect(index);
	spdlog::debug("Search {:p} index {} expandAndBackpropagate returning", (void*)this, index);
}

template <class Game, class Selector>
bool Search<Game, Selector>::done() const {
	return (getNCompletions() == getNIterations()) && (getNActiveTasks() == 0);
}

template <class Game, class Selector>
void Search<Game, Selector>::incrementNCompletions() {
	++m_n_completions;
	spdlog::debug("Search {:p} incrementNCompletions", (void*)this);
}

template <class Game, class Selector>
size_t Search<Game, Selector>::getNCompletions() const {
	return m_n_completions;
}

template <class Game, class Selector>
void Search<Game, Selector>::initialise(const Game& game) {

	std::random_device seeder;
	m_generator.seed(seeder());
	
	
	for(size_t index=0; index!=getBatchSize(); ++index) {
		m_games[index] = game;
		m_nodes[index] = &m_root;
		m_paused_nodes[index] = nullptr;
	}
}

template <class Game, class Selector>
Search<Game, Selector>::Search(
	const Game& game, 
	oaz::evaluator::Evaluator<Game>* evaluator, 
	oaz::thread_pool::ThreadPool* thread_pool,
	size_t batch_size, 
	size_t n_iterations):
	m_batch_size(batch_size),
	m_n_iterations(n_iterations),
	m_n_selections(0),
	m_n_completions(0),
	m_n_evaluation_requests(0),
	m_n_active_tasks(0),
	m_nodes(batch_size),
	m_paused_nodes(batch_size),
	m_games(batch_size),
	m_values(batch_size),
	m_policies(batch_size),
	m_evaluator(evaluator),
	m_noise_epsilon(0.),
	m_noise_alpha(1.),
	m_thread_pool(thread_pool),
	m_selection_tasks(boost::extents[batch_size]), 
	m_expansion_and_backpropagation_tasks(boost::extents[batch_size]){
	initialise(game);
}

template <class Game, class Selector>
Search<Game, Selector>::Search(
	const Game& game, 
	oaz::evaluator::Evaluator<Game>* evaluator, 
	oaz::thread_pool::ThreadPool* thread_pool,
	size_t batch_size, 
	size_t n_iterations,
	float noise_epsilon,
	float noise_alpha):
	m_batch_size(batch_size),
	m_n_iterations(n_iterations),
	m_n_selections(0),
	m_n_completions(0),
	m_n_evaluation_requests(0),
	m_n_active_tasks(0),
	m_nodes(batch_size),
	m_paused_nodes(batch_size),
	m_games(batch_size),
	m_values(batch_size),
	m_policies(batch_size),
	m_evaluator(evaluator),
	m_noise_epsilon(noise_epsilon),
	m_noise_alpha(noise_alpha),
	m_thread_pool(thread_pool),
	m_selection_tasks(boost::extents[batch_size]), 
	m_expansion_and_backpropagation_tasks(boost::extents[batch_size]){
	initialise(game);
}

template <class Game, class Selector>
typename Search<Game, Selector>::Node* Search<Game, Selector>::backpropagateNode(
	Node* node, Game& game, float normalised_score) {
	spdlog::debug("Search {:p} node {:p} backpropagateNode", (void*)this, (void*)node);
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
	spdlog::debug("Search {:p} node {:p} backpropagateNode returning", (void*)this, (void*)node);
	return node;
}

template <class Game, class Selector>
size_t Search<Game, Selector>::getBatchSize() const {
	return m_batch_size;
}

template <class Game, class Selector>
void Search<Game, Selector>::handleCreatedTask() {
	m_n_active_tasks++;
}

template <class Game, class Selector>
void Search<Game, Selector>::handleFinishedTask() {
	m_n_active_tasks--;
	if(done())
		m_condition.notify_one();
}

template <class Game, class Selector>
size_t Search<Game, Selector>::getNActiveTasks() const {
	return m_n_active_tasks;
}

template<class Game, class Selector>
typename Search<Game, Selector>::Node* Search<Game, Selector>::getNode(size_t index) {
	return m_nodes[index];
}

template <class Game, class Selector>
void Search<Game, Selector>::search() {

	spdlog::info("Search {:p} searching", (void*)this);
	for(size_t i=0; i!=getBatchSize(); ++i)
		maybeSelect(i);

	std::unique_lock<std::mutex> lock(m_mutex);
	m_condition.wait(lock, [this]{ return done(); });
	spdlog::info("Search {:p} returning", (void*)this);
}


template <class Game, class Selector>
void Search<Game, Selector>::setNode(size_t index, Node* node) {
	m_nodes[index] = node;
}

template <class Game, class Selector>
typename Search<Game, Selector>::Node* Search<Game, Selector>::getTreeRoot() {
	return &m_root;
}


template <class Game, class Selector>
typename Search<Game, Selector>::Move Search<Game, Selector>::getBestMove() {
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

template <class Game, class Selector>
void Search<Game, Selector>::getVisitCounts(Policy& move_counts) {
	for(size_t i=0; i!= m_root.getNChildren(); ++i) {
		Node* node = m_root.getChild(i);
		move_counts[node->getMove()] = node->getNVisits();
	}
}

template <class Game, class Selector>
Search<Game, Selector>::~Search() {}
