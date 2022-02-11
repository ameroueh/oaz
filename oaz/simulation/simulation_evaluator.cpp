#include "oaz/simulation/simulation_evaluator.hpp"

oaz::simulation::SimulationEvaluation::SimulationEvaluation(float value): m_value(value) {}

float oaz::simulation::SimulationEvaluation::GetValue() const { return m_value; }

float oaz::simulation::SimulationEvaluation::GetPolicy(size_t move) const { return 0.; }

std::unique_ptr<oaz::evaluator::Evaluation> oaz::simulation::SimulationEvaluation::Clone() const {
  return std::make_unique<SimulationEvaluation>(*this);
}

oaz::simulation::SimulationEvaluator::SimulationEvaluator(
    std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool)
    : m_thread_pool(std::move(thread_pool)) {}

void oaz::simulation::SimulationEvaluator::RequestEvaluation(
    oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
    oaz::thread_pool::Task* task) {
  std::vector<size_t> available_moves;
  std::unique_ptr<oaz::games::Game> game_copy = game->Clone();
  *evaluation = std::move(std::make_unique<SimulationEvaluation>(Simulate(game_copy.get())));
  m_thread_pool->enqueue(task);
}

float oaz::simulation::SimulationEvaluator::Simulate(oaz::games::Game* game) {
  std::vector<size_t> available_moves;
  size_t current_player = game->GetCurrentPlayer();
  while (!game->IsFinished()) {
    game->GetAvailableMoves(&available_moves);
    std::uniform_int_distribution<size_t> dis(0, available_moves.size() - 1);
    size_t random_move_index = dis(m_generator);
    auto random_move = available_moves[random_move_index];
    game->PlayMove(random_move);
  }
  float score = game->GetScore();

  return (current_player == 0) ? score : score * -1.0F;
}
