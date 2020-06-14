#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/model.hpp"
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
using SharedModelPointer = std::shared_ptr<Model>;
using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool>;
using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;

using namespace oaz::mcts;

static const size_t N_SIMULATIONS_PER_MOVE = 40;
static const size_t SEARCH_BATCH_SIZE = 16;
static const size_t N_GAMES = 5;
static const size_t N_WORKERS = 5;
	
namespace oaz::az {
	TEST (Instantiation, Default) {
		SharedModelPointer model(new Model());
		model->Load(
			"frozen_model.pb",
			"value",
			"policy"
		);
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SharedSearchPoolPointer shared_search_pool_ptr(
			new SearchPool(evaluator, 1.));
		::SelfPlay self_play(
			"games.h5",
			evaluator,
			shared_search_pool_ptr,
			1,
			1,
			1,
			1
		);
	}
	
	TEST (PlayGames, Default) {
		SharedModelPointer model(new Model());
		model->Load(
			"frozen_model.pb",
			"value",
			"policy"
		);
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SharedSearchPoolPointer shared_search_pool_ptr(
			new SearchPool(evaluator, 1.));
		::SelfPlay self_play(
			"games.h5",
			evaluator,
			shared_search_pool_ptr,
			N_GAMES, 
			N_SIMULATIONS_PER_MOVE,
			SEARCH_BATCH_SIZE,
			1
		);

		self_play.playGames(); 
	}

	// Need to test sampleMove
	
	TEST (PlayGames, MultiThreaded) {
		SharedModelPointer model(new Model());
		model->Load(
			"frozen_model.pb",
			"value",
			"policy"
		);
		SharedEvaluatorPointer evaluator(
			new Evaluator(model, 64)
		);
		SharedSearchPoolPointer shared_search_pool_ptr(
			new SearchPool(evaluator, 1.));
		::SelfPlay self_play(
			"games.h5",
			evaluator,
			shared_search_pool_ptr,
			N_GAMES, 
			N_SIMULATIONS_PER_MOVE,
			SEARCH_BATCH_SIZE,
			N_WORKERS
		);

		self_play.playGames(); 
	}
}
