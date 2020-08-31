#ifndef __EVALUATOR_HPP__
#define __EVALUATOR_HPP__

#include "oaz/thread_pool/thread_pool.hpp"


namespace oaz::evaluator {
	
	template <class Game>
	class Evaluator {
		public:
			virtual void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				oaz::thread_pool::Task*
			) = 0;
			virtual ~Evaluator() {}
	};
}
#endif
