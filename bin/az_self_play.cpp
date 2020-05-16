#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/connect_four.hpp" 
#include "oaz/mcts/az_search.hpp" 
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/az/self_play.hpp"
#include "oaz/logging/monitor.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>

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
namespace po = boost::program_options;


int main(int argc, char* argv[])
{
	po::variables_map variables_map;
  	
	try {
		po::options_description description("Allowed options for az_self_play");
		description.add_options()
			("help", "produce this help message")
			("n-simulations-per-move", po::value<size_t>()->required(), "number of simulations to perform for search")
			("search-batch-size", po::value<size_t>()->required(), "number of concurrent workers for each search")
			("n-games", po::value<size_t>()->required(), "number of games to play")
			("n-search-workers", po::value<size_t>()->required(), "number workers in the search pool")
			("n-game-workers", po::value<size_t>()->required(), "number workers performing self-play")
			("evaluator-batch-size", po::value<size_t>()->required(), "size of batches sent for evaluation")
			("model-path", po::value<std::string>()->required(), "load model from file 'arg'");
			("games-path", po::value<std::string>()->required(), "save games to file 'arg'");

		po::store(po::parse_command_line(argc, argv, description), variables_map);

		if (variables_map.count("help")) {
			std::cout << description << std::endl;
			return true;
		}
		po::notify(variables_map);
	
	} 
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	
	SharedModelPointer model(new Model());
	model->Load(variables_map["model-path"].as<std::string>());
	SharedEvaluatorPointer evaluator(new Evaluator(model, variables_map["evaluator-batch-size"].as<size_t>()));
	SharedSearchPoolPointer  search_pool(
		new SearchPool(evaluator, variables_map["n-search-workers"].as<size_t>())
	);

	SharedSelfPlayPointer self_play(
		new ::SelfPlay(
				variables_map["games-path"].as<std::string>(),
				evaluator,
				search_pool,
				variables_map["n-games"].as<size_t>(), 
				variables_map["n-simulations-per-move"].as<size_t>(), 
				variables_map["search-batch-size"].as<size_t>(),
				variables_map["n-games-workers"].as<size_t>()
		)
	);
	Monitor monitor(self_play, evaluator, search_pool);
	self_play->playGames();
}
