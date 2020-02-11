#include "oaz/random/random_evaluator.hpp"

#include <random>
#include <stack>

using namespace oaz::random;

template <size_t n_moves>
float Array1D<n_moves>::operator()(size_t index) const {
	return (*this)[index];
}

template <class Game, class GamesContainer>
RandomEvaluator<Game, GamesContainer>::RandomEvaluator(GamesContainer*games): m_games(games) {
	initialise();
}


template <class Game, class GamesContainer>
Game& RandomEvaluator<Game, GamesContainer>::getGame(size_t index) {
	return (*m_games)[index];
}

template <class Game, class GamesContainer>
void RandomEvaluator<Game, GamesContainer>::evaluate() {
	for(size_t i=0; i!=(*m_games).size(); ++i) {
		m_scores[i] = simulate((*m_games)[i]);
	}
}

template <class Game, class GamesContainer>
float RandomEvaluator<Game, GamesContainer>::simulate(Game& game) {
	
	std::stack<typename Game::move_t> played_moves;
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

template <class Game, class GamesContainer> 
float RandomEvaluator<Game, GamesContainer>::getValue(size_t index) const {
	return m_scores[index];
}

template <class Game, class GamesContainer>
typename RandomEvaluator<Game, GamesContainer>::PolicyType& RandomEvaluator<Game, GamesContainer>::getPolicy(size_t index) {
	return m_policies[index];
}

template <class Game, class GamesContainer>
void RandomEvaluator<Game, GamesContainer>::initialise() {
	size_t size = (*m_games).size();
	m_scores.resize(size, 0.);
	m_policies.resize(size, PolicyType());
}
