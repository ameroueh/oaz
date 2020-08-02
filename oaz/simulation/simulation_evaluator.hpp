#ifndef __SIMULATION_EVALUATOR_H__
#define __SIMULATION_EVALUATOR_H__

#include <array>
#include <string>
#include <vector>

#include <random>
#include "stdint.h"
#include <array>
#include "oaz/queue/queue.hpp"

namespace oaz::random {

	template <class Game, class Notifier>
	class SimulationEvaluator {
		public:
			SimulationEvaluator();
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				Notifier
			);
			void forceEvaluation();
		private:
			float simulate(Game&);
			std::mt19937 m_generator;
	};
}

#include "simulation_evaluator_impl.cpp"
#endif 
