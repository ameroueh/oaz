#ifndef OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_
#define OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_

#include <memory>
#include <random>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::simulation {

class SimulationEvaluation : public oaz::evaluator::Evaluation {
  public:
    SimulationEvaluation() = default;
    SimulationEvaluation(float);
    float GetValue() const;
    float GetPolicy(size_t) const;

    std::unique_ptr<Evaluation> Clone() const;

  private:
    float m_value;
};

class SimulationEvaluator : public oaz::evaluator::Evaluator {
 public:
  explicit SimulationEvaluator(std::shared_ptr<oaz::thread_pool::ThreadPool>);
  void RequestEvaluation(oaz::games::Game* game,
		  	 std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
                         oaz::thread_pool::Task* task) override;

 private:
  float Simulate(oaz::games::Game*);
  std::mt19937 m_generator;

  std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
};
}  // namespace oaz::simulation

#endif  // OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_
