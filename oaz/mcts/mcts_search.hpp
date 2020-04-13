#ifndef __MCTS_SEARCH_HPP__
#define __MCTS_SEARCH_HPP__

#include "oaz/mcts/search.hpp"

namespace oaz::mcts {

	template <class Game, class Evaluator>
	using MCTSSearch = Search<Game, Evaluator, UCTSelector<SearchNode<typename Game::Move>>>;
}

#endif
