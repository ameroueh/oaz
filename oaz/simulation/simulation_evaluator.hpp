#ifndef OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_
#define OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_

#include <memory>
#include <random>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::simulation {

class SimulationEvaluator : public oaz::evaluator::Evaluator {
 public:
  SimulationEvaluator(std::shared_ptr<oaz::thread_pool::ThreadPool>);
  void RequestEvaluation(oaz::games::Game*, float*,
                         boost::multi_array_ref<float, 1>,
                         oaz::thread_pool::Task*);

 private:
  float Simulate(oaz::games::Game&);
  std::mt19937 m_generator;

  std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
};
}  // namespace oaz::simulation

#endif OAZ_SIMULATION_SIMULATION_EVALUATOR_HPP_
