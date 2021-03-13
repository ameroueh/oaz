#ifndef OAZ_EVALUATOR_EVALUATOR_HPP_
#define OAZ_EVALUATOR_EVALUATOR_HPP_

#include "boost/multi_array.hpp"

#include "oaz/games/game.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::evaluator {

class Evaluator {
 public:
  virtual void RequestEvaluation(oaz::games::Game*, float*,
                                 boost::multi_array_ref<float, 1>,
                                 oaz::thread_pool::Task*) = 0;
  virtual ~Evaluator() {}
};
}  // namespace oaz::evaluator
#endif // OAZ_EVALUATOR_EVALUATOR_HPP_
