#ifndef __SELF_PLAY_HPP__
#define __SELF_PLAY_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <random>
#include <string>

#include "H5Cpp.h"

#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/mutex/mutex.hpp"

namespace oaz::az {
	template <class Game, class Evaluator, class SearchPool>
	class SelfPlay {

		/* TEST_FRIENDS; */

		public:	
			using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
			SelfPlay(
				std::string,
				SharedEvaluatorPointer, 
				SharedSearchPoolPointer,
				size_t,
				size_t,
				size_t,
				size_t
			);
			
			std::string getStatus();
			void playGames();

		private:
			void initialise();
		
			void playGame(size_t, size_t, size_t);
			void work();
			void normaliseVisitCounts(typename Game::Policy&);
			typename Game::Move sampleMove(
				typename Game::Policy&,
				std::vector<typename Game::Move>*
			);

			SharedSearchPoolPointer m_search_pool;
			SharedEvaluatorPointer m_evaluator;

			size_t m_n_games;
			size_t m_n_simulations_per_move;
			size_t m_search_batch_size;
			size_t m_n_workers;
			std::atomic<size_t> m_counter;

			std::mt19937 m_generator;

			H5::H5File m_file;
			oaz::mutex::SpinlockMutex m_lock;
	};

}

#include "oaz/az/self_play_impl.cpp"
#endif
