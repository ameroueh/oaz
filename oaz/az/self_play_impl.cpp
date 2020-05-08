#include "oaz/az/self_play.hpp"

#include <algorithm>
#include <iostream>
#include <stack>


template <class Game, class Evaluator, class SearchPool, class Trainer>
oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::SelfPlay(
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size,
	size_t n_workers):
	m_search_pool(search_pool),
	m_evaluator(evaluator),
	m_trainer(nullptr),
	m_n_games(n_games), 
	m_n_simulations_per_move(n_simulations_per_move),
	m_search_batch_size(search_batch_size), 
	m_n_workers(n_workers),
	m_counter(0)
	{
		initialise();
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::SelfPlay(
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool,
	SharedTrainerPointer trainer,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size,
	size_t n_workers):
	m_search_pool(search_pool),
	m_evaluator(evaluator),
	m_trainer(trainer),
	m_n_games(n_games), 
	m_n_simulations_per_move(n_simulations_per_move),
	m_search_batch_size(search_batch_size), 
	m_n_workers(n_workers),
	m_counter(0) {
		initialise();
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::initialise() {}

template <class Game, class Evaluator, class SearchPool, class Trainer>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::playGame(
	size_t n_simulations_per_move,
	size_t search_batch_size
) {

	Game game;

	std::vector<typename Game::Board> boards;
	std::vector<typename Game::Policy> policies;
	std::vector<typename Game::Value> values;


	size_t n_moves = 0;
	while(!game.Finished()) {

		boards.push_back(game.getBoard());

		AZSearch<Game, Evaluator> search(
			game, 
			m_evaluator,
			search_batch_size, 
			n_simulations_per_move,
			0.25,
			1.0
		);

		m_search_pool->performSearch(&search);
		
		typename Game::Policy policy;
		search.getVisitCounts(policy);
		normaliseVisitCounts(policy);
	
		std::vector<typename Game::Move>* available_moves = game.availableMoves();
		typename Game::Move move = sampleMove(policy, available_moves);

		policies.push_back(policy);
		
		game.playMove(move);
		++n_moves;
	}

	float score = game.score(); // 1 if first player won, -1 if second player won, 0 for a draw
	std::cout << "Score: " << score << " game length " << n_moves << std::endl;

	for(size_t i=0; i!=n_moves; ++i)
		values.push_back(score);

	if (m_trainer) {
		for(size_t i=0; i!=n_moves; ++i) {
			m_trainer->addTrainingExample(
				&boards[i],
				&values[i],
				&policies[i]
			);
		}

	}
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::normaliseVisitCounts(typename Game::Policy& visit_counts) {
	float total = 0.;
	for(size_t i=0; i!=visit_counts.size(); ++i)
		total += visit_counts[i];
	
	for(size_t i=0; i!=visit_counts.size(); ++i)
		visit_counts[i] /= total;
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
typename Game::Move oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::sampleMove(typename Game::Policy& visit_counts, std::vector<typename Game::Move>* available_moves) {

	std::vector<float> available_moves_visit_counts;
	for(auto move : *available_moves)
		available_moves_visit_counts.push_back(visit_counts[move]);
	
	std::discrete_distribution<size_t> distribution(
		available_moves_visit_counts.begin(), 
		available_moves_visit_counts.end()
	);

	size_t move_index = distribution(m_generator);
	return (*available_moves)[move_index];
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::work() {
	while(m_counter++ < m_n_games) {
		playGame(m_n_simulations_per_move, m_search_batch_size);
	}
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::playGames() {

	vector<std::thread> workers;
	
	for(size_t i = 0; i != m_n_workers; ++i) {
		workers.push_back(
			std::thread(
				&SelfPlay<Game, Evaluator, SearchPool, Trainer>::work,
				this
			)
		);
	}

	for(size_t i=0; i!=m_n_workers; ++i)
		workers[i].join();
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
std::string oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::getStatus() {
	size_t counter = std::min<size_t>(m_counter, m_n_games);
	return "Self-play progress: " + std::to_string(counter) + "/" + to_string(m_n_games);
}
