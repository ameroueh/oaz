#ifndef OAZ_EVALUATOR_UNIFORM_POLICY_EVALUATOR_HPP_
#define OAZ_EVALUATOR_UNIFORM_POLICY_EVALUATOR_HPP_

#include <memory>
#include <random>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::evaluator {

class UniformPolicyEvaluation : public oaz::evaluator::Evaluation {
  public:
    UniformPolicyEvaluation(size_t);
    virtual float GetValue() const;
    virtual float GetPolicy(size_t) const;

    std::unique_ptr<Evaluation> Clone() const;

  private:
    size_t m_n_moves;
};

class UniformPolicyEvaluator : public oaz::evaluator::Evaluator {
 public:
  explicit UniformPolicyEvaluator(std::shared_ptr<oaz::thread_pool::ThreadPool>);
  void RequestEvaluation(oaz::games::Game* game, float* value,
		         std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
                         oaz::thread_pool::Task* task) override;

 private:
  std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
};
}  // namespace oaz::evaluator

#endif  // OAZ_EVALUATOR_UNIFORM_POLICY_EVALUATOR_HPP_
