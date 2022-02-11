#ifndef OAZ_EVALUATOR_EVALUATOR_HPP_
#define OAZ_EVALUATOR_EVALUATOR_HPP_

#include "boost/multi_array.hpp"
#include "oaz/games/game.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::evaluator {

class Evaluation {
  public:
    virtual float GetValue() const = 0;
    virtual float GetPolicy(size_t) const = 0;

    virtual std::unique_ptr<Evaluation> Clone() const = 0;

    virtual ~Evaluation() {}
    Evaluation() = default;
    Evaluation(const Evaluation&) = default;
    Evaluation(Evaluation&&) = default;
    Evaluation& operator=(const Evaluation&) = default;
    Evaluation& operator=(Evaluation&&) = default;

};

class Evaluator {
 public:
  virtual void RequestEvaluation(oaz::games::Game*, std::unique_ptr<Evaluation>*, oaz::thread_pool::Task*) = 0;

  virtual ~Evaluator() {}
  Evaluator() = default;
  Evaluator(const Evaluator&) = default;
  Evaluator(Evaluator&&) = default;
  Evaluator& operator=(const Evaluator&) = default;
  Evaluator& operator=(Evaluator&&) = default;
};
}  // namespace oaz::evaluator
#endif  // OAZ_EVALUATOR_EVALUATOR_HPP_
