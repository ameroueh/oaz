#ifndef __SIMULATION_EVALUATOR_H__
#define __SIMULATION_EVALUATOR_H__

#include <array>
#include <string>
#include <vector>

#include <random>
#include "stdint.h"
#include <array>
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/queue/queue.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::random {

	template <class Game>
	class SimulationEvaluator : public oaz::evaluator::Evaluator<Game> {
		public:
			SimulationEvaluator(oaz::thread_pool::ThreadPool*);
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				oaz::thread_pool::Task*
			);

			void forceEvaluation();
		private:
			float simulate(Game&);
			std::mt19937 m_generator;
			oaz::thread_pool::ThreadPool* m_thread_pool;
	};
}

#include "simulation_evaluator_impl.cpp"
#endif 
