#include "oaz/az/self_play.hpp"
#include <iostream>


template <class Game, class Evaluator, class SearchPool>
oaz::az::SelfPlay<Game, Evaluator, SearchPool>::SelfPlay(
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool): 
	m_search_pool(search_pool),
	m_evaluator(evaluator) {
		initialise();
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::initialise() {}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::playGame(
	size_t n_simulations_per_move,
	size_t search_batch_size
) {

	Game game;
	
	while(!game.Finished()) {
		AZSearch<Game, Evaluator> search(
			game, 
			m_evaluator,
			search_batch_size, 
			n_simulations_per_move
		);

		m_search_pool->performSearch(&search);

		typename Game::Move best_move = search.getBestMove();

		game.playMove(best_move);
	}
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::playGames(
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size
) {
	for(size_t i=0; i != n_games; ++i) {
		playGame(n_simulations_per_move, search_batch_size);
	}
}
