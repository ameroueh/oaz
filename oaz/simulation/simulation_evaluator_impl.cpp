#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/queue/queue.hpp"

#include <random>
#include <stack>

using namespace oaz::random;

template <class Game, class Notifier>
SimulationEvaluator<Game, Notifier>::SimulationEvaluator() {
}

template <class Game, class Notifier>
void SimulationEvaluator<Game, Notifier>::requestEvaluation(
	Game* game, 
	typename Game::Value* value,
	typename Game::Policy* policy,
	Notifier notifier) {

	*value = simulate(*game);
	notifier();
}

template <class Game, class Notifier>
float SimulationEvaluator<Game, Notifier>::simulate(Game& game) {

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

template <class Game, class Notifier>
void SimulationEvaluator<Game, Notifier>::forceEvaluation() {
}
