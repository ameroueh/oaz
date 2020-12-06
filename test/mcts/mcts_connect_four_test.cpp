#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/mcts/search.hpp"
#include "oaz/mcts/selection.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

size_t GetBestMove(Search& search) {
	auto tree_root = search.GetTreeRoot();
	size_t best_move = 0;
	size_t best_n_visits = 0;
	for(size_t i=0; i != tree_root->GetNChildren(); ++i) {
		auto child = tree_root->GetChild(i);
		size_t n_visits = child->GetNVisits();
		if (n_visits >= best_n_visits) {
			best_n_visits = n_visits;
			best_move = child->GetMove();
		}
	}
	return best_move;
}

TEST (ForcedMoves, Scenario1) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(4);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
	UCTSelector selector;
	ConnectFour game;
	game.PlayFromString("5443233212"); // Expect best move to be 2
	Search search(game, selector, evaluator, pool, 4, 10000);

	ASSERT_EQ(GetBestMove(search), 2);
}

TEST (ForcedMoves, Scenario2) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(4);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
	UCTSelector selector;
	ConnectFour game;
	game.PlayFromString("4330011115"); // Expect best move to be 2
	
	Search search(game, selector, evaluator, pool, 4, 10000);
	
	ASSERT_EQ(GetBestMove(search), 2);
}

TEST (ForcedMoves, Scenario3) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(4);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
	UCTSelector selector;
	ConnectFour game;
	game.PlayFromString("433001111"); // Expect best move to be 2
	Search search(game, selector, evaluator, pool, 4, 10000);

	ASSERT_EQ(GetBestMove(search), 2);
}
