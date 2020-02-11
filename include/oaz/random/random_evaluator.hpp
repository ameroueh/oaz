#ifndef __RANDOM_EVALUATOR_H__
#define __RANDOM_EVALUATOR_H__

#include <array>
#include <string>
#include <vector>

#include <random>
#include "stdint.h"
#include <array>

namespace oaz::random {

	template <size_t n_moves>
	class Array1D: public std::array<float, n_moves> {
		public:
			float operator()(size_t) const;
	};

	template <class Game, class GamesContainer>
	class RandomEvaluator {
		public:
			using PolicyType = Array1D<Game::n_moves>;

			RandomEvaluator(GamesContainer*);
			void evaluate();
			Game& getGame(size_t);
			float getValue(size_t) const;
			PolicyType& getPolicy(size_t);

		private:
			void initialise();
			float simulate(Game&);
			GamesContainer* m_games;
			std::mt19937 m_generator;
			std::vector<float> m_scores;
			std::vector<PolicyType> m_policies;
	};

}

#include "random_evaluator_impl.cpp"
#endif // __RANDOM_EVALUATOR_H__
