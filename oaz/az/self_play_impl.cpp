#include "oaz/az/self_play.hpp"
#include <iostream>
#include <stack>


template <class Game, class Evaluator, class SearchPool, class Trainer>
oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::SelfPlay(
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool): 
	m_search_pool(search_pool),
	m_evaluator(evaluator),
	m_trainer(nullptr) {
		initialise();
}

template <class Game, class Evaluator, class SearchPool, class Trainer>
oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::SelfPlay(
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool,
	SharedTrainerPointer trainer): 
	m_search_pool(search_pool),
	m_evaluator(evaluator),
	m_trainer(trainer) {
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
			n_simulations_per_move
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
void oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>::playGames(
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size
) {
	for(size_t i=0; i != n_games; ++i) {
		std::cout << "Playing game " << i << std::endl;
		playGame(n_simulations_per_move, search_batch_size);
	}
}
