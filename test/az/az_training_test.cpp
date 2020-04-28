#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tensorflow/core/framework/tensor.h"


#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/neural_network/nn_trainer.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/az/self_play.hpp"

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
using Trainer = NNTrainer<Game>;
using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>;

using SharedModelPointer = std::shared_ptr<Model>;
using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
using SharedTrainerPointer = std::shared_ptr<Trainer>;

using namespace oaz::mcts;

static const size_t N_SIMULATIONS_PER_MOVE = 800;
static const size_t SEARCH_BATCH_SIZE = 16;
static const size_t N_GAMES = 50;
static const size_t N_WORKERS = 32;
	
void selfPlayGames(
	SharedEvaluatorPointer shared_evaluator_ptr, 
	SharedSearchPoolPointer shared_search_pool_ptr,
	SharedTrainerPointer shared_trainer_ptr,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size) {
	
	::SelfPlay self_play(
		shared_evaluator_ptr,
		shared_search_pool_ptr,
		shared_trainer_ptr
	);
	
	self_play.playGames(
		n_games, 
		n_simulations_per_move,
		search_batch_size
	); 

}

namespace oaz::az {
	TEST (AZTrainingTest, MultiThreaded) {
		SharedModelPointer model(new Model());
		model->Load("model");
		
		SharedModelPointer model2(new Model());
		model2->Load("model");

		SharedEvaluatorPointer evaluator(new Evaluator(model, 64));
		SharedSearchPoolPointer  search_pool(
			new SearchPool(evaluator, 0.4)
		);

		SharedTrainerPointer trainer(new Trainer(model2, 40, 20));

		vector<std::thread> workers;
		for(size_t i = 0; i != N_WORKERS; ++i) {
			workers.push_back(
				std::thread(
					&selfPlayGames,
					evaluator,
					search_pool,
					trainer,
					N_GAMES,
					N_SIMULATIONS_PER_MOVE,
					SEARCH_BATCH_SIZE
				)
			);
		}

		for(size_t i=0; i!=N_WORKERS; ++i) 
			workers[i].join();
	}
}
