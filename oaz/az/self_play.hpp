#ifndef __SELF_PLAY_HPP__
#define __SELF_PLAY_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <random>
#include <string>
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search_pool.hpp"

namespace oaz::az {
	template <class Game, class Evaluator, class SearchPool, class Trainer>
	class SelfPlay {

		/* TEST_FRIENDS; */

		public:	
			using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
			using SharedTrainerPointer = std::shared_ptr<Trainer>;
			SelfPlay(
				SharedEvaluatorPointer, 
				SharedSearchPoolPointer
			);
			
			SelfPlay(
				SharedEvaluatorPointer, 
				SharedSearchPoolPointer,
				SharedTrainerPointer
			);

			void playGames(size_t, size_t, size_t);
		
		private:
			void initialise();
			void playGame(size_t, size_t);
			void normaliseVisitCounts(typename Game::Policy&);
			typename Game::Move sampleMove(
				typename Game::Policy&,
				std::vector<typename Game::Move>*
			);

			SharedSearchPoolPointer m_search_pool;
			SharedEvaluatorPointer m_evaluator;
			SharedTrainerPointer m_trainer;

			std::mt19937 m_generator;
	};

}

#include "oaz/az/self_play_impl.cpp"
#endif
