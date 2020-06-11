#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/games/tic_tac_toe.hpp" 
#include "oaz/mcts/az_search.hpp" 
#include "oaz/mcts/az_search_pool.hpp"
#include "oaz/az/self_play.hpp"
#include "oaz/logging/monitor.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <boost/program_options.hpp>

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

/* class UnlockNotifier { */
/* 	public: */
/* 		UnlockNotifier(std::mutex* lock): m_lock(lock) {} */
/* 		UnlockNotifier() {} */
/* 		void operator()(){ m_lock->unlock(); } */
/* 	private: */
/* 		std::mutex* m_lock; */
/* }; */

using Game = TicTacToe;
using Move = typename TicTacToe::Move;
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
			("model-path", po::value<std::string>()->required(), "load model from file 'arg'")
			("value-op-name", po::value<std::string>()->required(), "value tensorflow op name")
			("policy-op-name", po::value<std::string>()->required(), "policy tensorflow op name")
			("n-simulations", po::value<size_t>()->default_value(0), "number of simulations to perform for search")
			("search-batch-size", po::value<size_t>()->default_value(1), "search batch size")
			("evaluator-batch-size", po::value<size_t>()->default_value(1), "evaluator batch size")
			("n-search-workers", po::value<size_t>()->default_value(1), "evaluator batch size")
			("moves", po::value<std::string>()->default_value(""), "moves to current position");

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

	size_t n_simulations = variables_map["n-simulations"].as<size_t>();
	size_t evaluator_batch_size = variables_map["evaluator-batch-size"].as<size_t>();
	size_t search_batch_size = variables_map["search-batch-size"].as<size_t>();
	
	SharedModelPointer model(new Model());
	model->Load(
		variables_map["model-path"].as<std::string>(),
		variables_map["value-op-name"].as<std::string>(),
		variables_map["policy-op-name"].as<std::string>()
	);
	
	SharedEvaluatorPointer evaluator(new Evaluator(model, evaluator_batch_size));

	Game game;
	game.playFromString(variables_map["moves"].as<std::string>());

	typename Game::Value value;
	typename Game::Policy policy;


	/* std::mutex lock; */
	/* lock.lock(); */
	oaz::queue::SafeQueue<size_t> dummy_queue;

	std::cout << "Performing evaluation" << std::endl;
	evaluator->requestEvaluation(
		&game,
		&value,
		&policy,
		SafeQueueNotifier(&dummy_queue, 0)
	);
	evaluator->forceEvaluation();
	std::cout << "Finished performing evaluation" << std::endl;

	typename Game::Policy visit_counts;
	if(n_simulations > 0) {
		std::cout << "Performing search" << std::endl;
		SharedSearchPoolPointer  search_pool(
			new SearchPool(evaluator, variables_map["n-search-workers"].as<size_t>())
		);
		AZSearch<Game, Evaluator> search(
			game, 
			evaluator,
			search_batch_size, 
			n_simulations,
			0.25,
			1.0
		);
		search_pool->performSearch(&search);
		search.getVisitCounts(visit_counts);
		std::cout << "Finished performing search" << std::endl;
	}

	std::cout << "Value " << value << std::endl;
	std::cout << "Neural network prior " << std::endl;

	for(size_t i=0; i!=Game::n_moves; ++i)
		std::cout << i << ": " << policy[i] << " ";
	std::cout << std::endl;

	if(n_simulations > 0) {
		std::cout << "Visit counts " << std::endl;
		for(size_t i=0; i!=Game::n_moves; ++i)
			std::cout << i << ": " << visit_counts[i] << " ";
		std::cout << std::endl;
	}
}
