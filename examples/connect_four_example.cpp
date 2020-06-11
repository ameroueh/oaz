#include <iostream>
#include <thread>

#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/mcts_search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Game = ConnectFour;
using Move = typename Game::Move;
using Node = SearchNode<Move>;
using Evaluator = RandomEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using GameSearch = MCTSSearch<Game, Evaluator>;

void search_until_done(GameSearch* search) {
	while(!search->done()) 
		search->work();
}


static size_t const NTHREADS=8;

int main() {
	std::shared_ptr<Evaluator> shared_evaluator_ptr(new Evaluator());
	
	Game game;
	GameSearch search (game, shared_evaluator_ptr, 16, 1000000); 

	std::vector<std::thread> threads;
	for(size_t i=0; i!=NTHREADS; ++i) 
		threads.push_back(std::thread(&search_until_done, &search));

	for(size_t i=0; i!=NTHREADS; ++i)
		threads[i].join();
	
	Node* root = search.getTreeRoot();		
	std::cout << serialiseTreeToJson<Move>(root) << std::endl;
}
