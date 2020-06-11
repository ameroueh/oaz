#ifndef __RANDOM_EVALUATOR_H__
#define __RANDOM_EVALUATOR_H__

#include <array>
#include <string>
#include <vector>

#include <random>
#include "stdint.h"
#include <array>
#include "oaz/queue/queue.hpp"

namespace oaz::random {

	/* template <size_t n_moves> */
	/* class Array1D: public std::array<float, n_moves> { */
	/* 	public: */
	/* 		float operator()(size_t) const; */
	/* }; */

	template <class Game, class Notifier>
	class RandomEvaluator {
		public:
			/* using PolicyType = Array1D<Game::n_moves>; */

			RandomEvaluator();
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				Notifier
			);
			void forceEvaluation();
			/* float getValue(size_t) const; */
			/* PolicyType getPolicy(size_t); */
			/* size_t getBatchSize() const; */

		private:
			/* size_t getSearchIndex(size_t) const; */
			/* void initialise(); */
			float simulate(Game&);
			/* GamesContainer* m_games; */
			std::mt19937 m_generator;
			/* std::vector<float> m_scores; */
			/* std::vector<PolicyType> m_policies; */

			/* std::vector<size_t> m_search_indices; */
			/* std::vector<oaz::queue::SafeQueue<size_t>* > m_completion_queues; */

			/* oaz::queue::SafeQueue<size_t> m_available_resources_q; */
	};
}

#include "random_evaluator_impl.cpp"
#endif // __RANDOM_EVALUATOR_H__
