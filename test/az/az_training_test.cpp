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
using Trainer = NNTrainer<Game>;
using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool, Trainer>;
using Monitor = oaz::logging::Monitor<::SelfPlay, Trainer, Evaluator, SearchPool>;

using SharedModelPointer = std::shared_ptr<Model>;
using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
using SharedTrainerPointer = std::shared_ptr<Trainer>;
using SharedSelfPlayPointer = std::shared_ptr<::SelfPlay>;

using namespace oaz::mcts;

static const size_t N_SIMULATIONS_PER_MOVE = 800;
static const size_t SEARCH_BATCH_SIZE = 16;
static const size_t N_GAMES = 640;
static const size_t N_WORKERS = 32;

namespace oaz::az {
	TEST (AZTrainingTest, MultiThreaded) {
		SharedModelPointer model(new Model());
		model->Load("model");
		
		SharedModelPointer model2(new Model());
		model2->Load("model");

		SharedEvaluatorPointer evaluator(new Evaluator(model, 64));
		SharedSearchPoolPointer  search_pool(
			new SearchPool(evaluator, (2. + 0.01) / 32.)
		);

		SharedTrainerPointer trainer(new Trainer(model2, 40, 20));
		
	
		SharedSelfPlayPointer self_play(
			new ::SelfPlay(
					evaluator,
					search_pool,
					trainer,
					N_GAMES,
					N_SIMULATIONS_PER_MOVE,
					SEARCH_BATCH_SIZE,
					N_WORKERS
			)
		);
		
		Monitor monitor(self_play, trainer, evaluator, search_pool);
		
		self_play->playGames();
	}
}
