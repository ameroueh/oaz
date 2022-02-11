#ifndef OAZ_CACHE_CACHE_HPP_
#define OAZ_CACHE_CACHE_HPP_

#include <memory>

#include "boost/multi_array.hpp"
#include "oaz/games/game.hpp"
#include "oaz/evaluator/evaluator.hpp"


namespace oaz::cache {
class Cache {
 public:
  virtual bool Evaluate(const oaz::games::Game&,
		  	std::unique_ptr<oaz::evaluator::Evaluation>*) = 0;
  virtual void Insert(const oaz::games::Game&,
                      std::unique_ptr<oaz::evaluator::Evaluation>*) = 0;

  virtual void BatchInsert(
      boost::multi_array_ref<oaz::games::Game*, 1>,
      boost::multi_array_ref<std::unique_ptr<oaz::evaluator::Evaluation>*, 1>,
      size_t) = 0;

  virtual ~Cache() = default;
  Cache() = default;
  Cache(const Cache&) = default;
  Cache& operator=(const Cache&) = default;
  Cache(Cache&&) = default;
  Cache& operator=(Cache&&) = default;
};
}  // namespace oaz::cache
#endif  // OAZ_CACHE_CACHE_HPP_
