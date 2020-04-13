#ifndef __SELF_PLAY_HPP__
#define __SELF_PLAY_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <string>
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search_pool.hpp"

namespace oaz::az {
	template <class Game, class Evaluator, class SearchPool>
	class SelfPlay {

		/* TEST_FRIENDS; */

		public:	
			using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
			SelfPlay(
				SharedEvaluatorPointer, 
				SharedSearchPoolPointer
			);

			void playGames(size_t, size_t, size_t);
		
		private:
			void initialise();
			void playGame(size_t, size_t);

			SharedSearchPoolPointer m_search_pool;
			SharedEvaluatorPointer m_evaluator;
	};

}

#include "oaz/az/self_play_impl.cpp"
#endif
