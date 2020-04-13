#include <iostream>
#include <thread>

#include "oaz/games/connect_four.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"

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

static const size_t N_SEARCHES = 1;
static const size_t SEARCH_BATCH_SIZE = 64;
static const size_t N_ITERATIONS_PER_SEARCH = 1000000;
static const size_t EVALUATOR_BATCH_SIZE = 64;

void createAndPerformSearch(
	std::shared_ptr<Evaluator> shared_evaluator_ptr, 
	SearchPool* search_pool, 
	size_t batch_size, 
	size_t n_iterations) {
		Game game;
		GameSearch search(
			game, 
			shared_evaluator_ptr, 
			batch_size,
			n_iterations
		);
		search_pool->performSearch(&search);
}

int main() {
	
	std::shared_ptr<Evaluator> shared_evaluator_ptr(
		new Evaluator(EVALUATOR_BATCH_SIZE)
	);
	shared_evaluator_ptr->load_model("model");		
	SearchPool search_pool(shared_evaluator_ptr, 8.);

	std::vector<std::thread> t;
	for(size_t i=0; i!=N_SEARCHES; ++i)
		t.push_back(
			std::thread(
				&createAndPerformSearch, 
				shared_evaluator_ptr,
				&search_pool, 
				SEARCH_BATCH_SIZE, 
				N_ITERATIONS_PER_SEARCH
			)
		);
	
	for(size_t i=0; i!=N_SEARCHES; ++i)
		t[i].join();
}
