#include "oaz/random/random_evaluator.hpp"
#include "oaz/queue/queue.hpp"

#include <random>
#include <stack>

using namespace oaz::random;

/* template <size_t n_moves> */
/* float Array1D<n_moves>::operator()(size_t index) const { */
/* 	return (*this)[index]; */
/* } */

template <class Game, class Notifier>
RandomEvaluator<Game, Notifier>::RandomEvaluator() {
}


/* template <class Game, class Notifier> */
/* Game& RandomEvaluator<Game, Notifier>::getGame(size_t index) { */
/* 	return (*m_games)[index]; */
/* } */

template <class Game, class Notifier>
void RandomEvaluator<Game, Notifier>::requestEvaluation(
	Game* game, 
	typename Game::Value* value,
	typename Game::Policy* policy,
	Notifier notifier) {

	*value = simulate(*game);
	// Copy policy here?
		
	notifier();
	/* completion_queue->lock(); */
	/* completion_queue->push(search_index); */
	/* completion_queue->unlock(); */

}

/* template <class Game, class Notifier> */
/* size_t RandomEvaluator<Game, Notifier>::getSearchIndex(size_t index) const { */
/* 	return m_search_indices[index]; */
/* } */

template <class Game, class Notifier>
float RandomEvaluator<Game, Notifier>::simulate(Game& game) {

	std::stack<typename Game::Move> played_moves;
	while (!game.Finished()) {
		auto available_moves = game.availableMoves();
		size_t n_available_moves = available_moves->size();
		std::uniform_int_distribution<size_t> dis(0, n_available_moves - 1);
		size_t random_move_index = dis(m_generator);
		auto random_move = (*available_moves)[random_move_index];
		game.playMove(random_move);
		played_moves.push(random_move);

	}

	float score = game.score();

	while(!played_moves.empty()) {
		auto move = played_moves.top();
		played_moves.pop();
		game.undoMove(move);
	}

	return score;
}

/* template <class Game, class Notifier> */ 
/* float RandomEvaluator<Game, Notifier>::getValue(size_t index) const { */
/* 	return m_scores[index]; */
/* } */

/* template <class Game, class Notifier> */
/* typename RandomEvaluator<Game, Notifier>::PolicyType RandomEvaluator<Game, GamesContainer>::getPolicy(size_t index) { */
/* 	return m_policies[index]; */
/* } */

/* template <class Game, class Notifier> */
/* void RandomEvaluator<Game, Notifier>::initialise() { */
/* 	m_scores.resize(getBatchSize(), 0.); */
/* 	m_policies.resize(getBatchSize(), PolicyType()); */
/* 	m_search_indices.resize(getBatchSize(), 0); */
/* 	m_completion_queues.resize(getBatchSize(), nullptr); */
	
/* 	for(size_t index=0; index!=getBatchSize(); ++index) { */
/* 		m_available_resources_q.push(index); */
/* 	} */
/* } */

/* template <class Game, class Notifier> */
/* size_t RandomEvaluator<Game, Notifier>::getBatchSize() const { */
/* 	return m_games->size(); */
/* } */

/* template <class Game, class Notifier> */
/* size_t RandomEvaluator<Game, Notifier>::acquireResource(size_t search_index, oaz::queue::SafeQueue<size_t>* completion_queue) { */
/* 	m_available_resources_q.lock(); */
/* 	if(!m_available_resources_q.empty()) { */
/* 		size_t index = m_available_resources_q.front(); */
/* 		m_available_resources_q.pop(); */
/* 		m_available_resources_q.unlock(); */
/* 		m_completion_queues[index] = completion_queue; */
/* 		m_search_indices[index] = search_index; */
/* 		return index; */
/* 	} else { */
/* 		m_available_resources_q.unlock(); */
/* 	} */
/* } */

/* template <class Game, class Notifier> */
/* void RandomEvaluator<Game, Notifier>::releaseResource(size_t index) { */
/* 	m_available_resources_q.lock(); */
/* 	m_available_resources_q.push(index); */
/* 	m_available_resources_q.unlock(); */
/* } */

template <class Game, class Notifier>
void RandomEvaluator<Game, Notifier>::forceEvaluation() {
}
