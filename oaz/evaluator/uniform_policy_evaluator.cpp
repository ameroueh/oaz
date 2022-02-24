#include "oaz/evaluator/uniform_policy_evaluator.hpp"

oaz::evaluator::UniformPolicyEvaluation::UniformPolicyEvaluation(size_t n_moves): m_n_moves(n_moves) {}

float oaz::evaluator::UniformPolicyEvaluation::GetValue() const {
  return 0.;
}

float oaz::evaluator::UniformPolicyEvaluation::GetPolicy(size_t move) const {
  return 1.0F / m_n_moves;
}

std::unique_ptr<oaz::evaluator::Evaluation> oaz::evaluator::UniformPolicyEvaluation::Clone() const {
  return std::make_unique<oaz::evaluator::UniformPolicyEvaluation>(*this);
}

oaz::evaluator::UniformPolicyEvaluator::UniformPolicyEvaluator(
    std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool)
    : m_thread_pool(std::move(thread_pool)) {}

void oaz::evaluator::UniformPolicyEvaluator::RequestEvaluation(
    oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
    oaz::thread_pool::Task* task) {
  std::vector<size_t> available_moves;
  game->GetAvailableMoves(&available_moves);
  *evaluation = std::make_unique<UniformPolicyEvaluation>(
    available_moves.size()
  );
  m_thread_pool->enqueue(task);
}
