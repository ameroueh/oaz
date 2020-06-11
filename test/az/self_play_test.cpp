#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/az/self_play.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

/* #define TEST_FRIENDS \ */
/* 	friend class PlayGame_Default_Test; */

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = ConnectFour;
using Move = typename ConnectFour::Move;
using Node = SearchNode<Move>;
using Evaluator = NNEvaluator<Game, SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;
using SearchPool = AZSearchPool<Game, Evaluator>;
using Policy = typename Game::Policy;
using Board = typename Game::Board;
using Value = typename Game::Value;

class DummyTrainer {
	public:
		void addTrainingExample(Board*, Value*, Policy*) {}
};

using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool, DummyTrainer>;

using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;

using namespace oaz::mcts;

static const size_t N_SIMULATIONS_PER_MOVE = 40;
static const size_t SEARCH_BATCH_SIZE = 16;
static const size_t N_GAMES = 5;
static const size_t N_WORKERS = 5;
	
void selfPlayGames(
	SharedEvaluatorPointer shared_evaluator_ptr, 
	SharedSearchPoolPointer shared_search_pool_ptr,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size) {
	
	::SelfPlay self_play(
		shared_evaluator_ptr,
		shared_search_pool_ptr
	);
	
	self_play.playGames(
		n_games, 
		n_simulations_per_move,
		search_batch_size
	); 

}

namespace oaz::az {
	TEST (Instantiation, Default) {
		SharedEvaluatorPointer shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		SharedSearchPoolPointer shared_search_pool_ptr(
			new SearchPool(shared_evaluator_ptr, 1.));
		::SelfPlay self_play(
			shared_evaluator_ptr,
			shared_search_pool_ptr
		);
	}
	
	TEST (PlayGames, Default) {
		SharedEvaluatorPointer shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		SharedSearchPoolPointer shared_search_pool_ptr(
			new SearchPool(shared_evaluator_ptr, 1.));
		::SelfPlay self_play(
			shared_evaluator_ptr,
			shared_search_pool_ptr
		);

		self_play.playGames(
			N_GAMES, 
			N_SIMULATIONS_PER_MOVE,
			SEARCH_BATCH_SIZE
		); 
	}

	// Need to test sampleMove
	
	TEST (PlayGames, MultiThreaded) {
		std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator(64));
		shared_evaluator_ptr->load_model("model");
		std::shared_ptr<SearchPool>  shared_search_pool_ptr(
			new SearchPool(shared_evaluator_ptr, 1.));

		vector<std::thread> workers;
		for(size_t i = 0; i != N_WORKERS; ++i) {
			workers.push_back(
				std::thread(
					&selfPlayGames,
					shared_evaluator_ptr,
					shared_search_pool_ptr,
					1,
					N_SIMULATIONS_PER_MOVE,
					SEARCH_BATCH_SIZE
				)
			);
		}

		for(size_t i=0; i!=N_WORKERS; ++i) 
			workers[i].join();
	}
}
