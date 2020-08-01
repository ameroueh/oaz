#include <iostream>
#include <thread>

#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/az/self_play.hpp"

#include "tensorflow/core/framework/tensor.h"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;
using namespace oaz::nn;
using namespace tensorflow;

using Game = ConnectFour;
using Move = typename Game::Move;
using Node = SearchNode<Move>;
using Evaluator = NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using GameSearch = AZSearch<Game, Evaluator>;
using SearchPool = AZSearchPool<Game, Evaluator>;
using SelfPlay = oaz::az::SelfPlay<Game, Evaluator, SearchPool>;

using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;

static const size_t N_WORKERS = 8;
static const size_t N_GAMES_PER_WORKER = 16;
static const size_t SEARCH_BATCH_SIZE = 4;
static const size_t N_SIMULATIONS_PER_MOVE = 100;
static const size_t EVALUATOR_BATCH_SIZE = 4;

void selfPlayGames(
	SharedEvaluatorPointer shared_evaluator_ptr, 
	SharedSearchPoolPointer shared_search_pool_ptr,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size) {
	
	SelfPlay self_play(
		shared_evaluator_ptr,
		shared_search_pool_ptr
	);
	
	self_play.playGames(
		n_games, 
		n_simulations_per_move,
		search_batch_size
	); 

}

int main() {
	std::shared_ptr<Evaluator> shared_evaluator_ptr(
		new Evaluator(EVALUATOR_BATCH_SIZE)
	);
	shared_evaluator_ptr->load_model("model");
	std::shared_ptr<SearchPool>  shared_search_pool_ptr(
		new SearchPool(shared_evaluator_ptr, 1.)
	);

	vector<std::thread> workers;
	for(size_t i = 0; i != N_WORKERS; ++i) {
		workers.push_back(
			std::thread(
				&selfPlayGames,
				shared_evaluator_ptr,
				shared_search_pool_ptr,
				N_GAMES_PER_WORKER,
				N_SIMULATIONS_PER_MOVE,
				SEARCH_BATCH_SIZE
			)
		);
	}

	for(size_t i=0; i!=N_WORKERS; ++i) 
		workers[i].join();
}
