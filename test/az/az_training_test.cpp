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
#include "oaz/logging/monitor.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = ConnectFour;
using Move = typename ConnectFour::Move;
using Node = SearchNode<Move>;
using Model = oaz::nn::Model;
using Evaluator = NNEvaluator<Game, SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;
using SearchPool = AZSearchPool<Game, Evaluator>;
using Policy = typename Game::Policy;
using Board = typename Game::Board;
using Value = typename Game::Value;
using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool> ;
using Monitor = oaz::logging::Monitor<::SelfPlay, Evaluator, SearchPool>;

using SharedModelPointer = std::shared_ptr<Model>;
using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
using SharedSelfPlayPointer = std::shared_ptr<::SelfPlay>;

using namespace oaz::mcts;

static const size_t N_SIMULATIONS_PER_MOVE = 800;
static const size_t SEARCH_BATCH_SIZE = 16;
static const size_t N_GAMES_PER_EPOCH = 200;
static const size_t N_EPOCHS = 50;
static const size_t N_WORKERS = 64;

namespace oaz::az {
	
	/* void playFromString(ConnectFour* game, std::string sMoves) { */
	/* 	for(char& c : sMoves) */
	/* 		game->playMove(c - '0'); */
	/* } */

	/* void benchmarkModel(SharedModelPointer model, SharedSearchPoolPointer search_pool, SharedEvaluatorPointer evaluator) { */
	/* 	Game game; */
	/* 	playFromString(&game, moves); */


	/* } */

	TEST (AZTrainingTest, MultiThreaded) {
		SharedModelPointer model(new Model());
		model->Load("model");
		
		SharedEvaluatorPointer evaluator(new Evaluator(model, 96));
		SharedSearchPoolPointer  search_pool(
			new SearchPool(evaluator, 6)
		);

		for(size_t i=0; i!=N_EPOCHS; ++i) {
			std::cout << "Epoch " << i << "; generating " << N_GAMES_PER_EPOCH << " games" << std::endl;
			SharedSelfPlayPointer self_play(
				new ::SelfPlay(
						"output.h5",
						evaluator,
						search_pool,
						N_GAMES_PER_EPOCH,
						N_SIMULATIONS_PER_MOVE,
						SEARCH_BATCH_SIZE,
						N_WORKERS
				)
			);
			Monitor monitor(self_play, evaluator, search_pool);
			self_play->playGames();

			model->Checkpoint("model_" + std::to_string(i));

			std::cout << "Benchmarking model" << std::endl;
		}	
	}
}
