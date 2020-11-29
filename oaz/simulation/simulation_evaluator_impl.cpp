#include "oaz/simulation/simulation_evaluator.hpp"

using namespace oaz::simulation;

SimulationEvaluator::SimulationEvaluator(
	std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool):
	m_thread_pool(thread_pool) {}

void SimulationEvaluator::RequestEvaluation(
	oaz::games::Game* game, 
	float* value,
	boost::multi_array_ref<float, 1> policy,
	oaz::thread_pool::Task* task) {

	std::unique_ptr<oaz::games::Game> game_copy = game->Clone();
	*value = Simulate(*game_copy);
	m_thread_pool->enqueue(task);
}

float SimulationEvaluator::Simulate(oaz::games::Game& game) {

	std::vector<size_t> available_moves;
	size_t current_player = game.GetCurrentPlayer();
	while (!game.IsFinished()) {
		game.GetAvailableMoves(available_moves);
		std::uniform_int_distribution<size_t> dis(
			0, 
			available_moves.size() - 1
		);
		size_t random_move_index = dis(m_generator);
		auto random_move = available_moves[random_move_index];
		game.PlayMove(random_move);
	}
	float score = game.GetScore();

	return (current_player == 0) ? score : score * -1.;
}
