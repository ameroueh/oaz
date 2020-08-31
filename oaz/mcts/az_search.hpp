#ifndef __AZ_SEARCH_HPP__
#define __AZ_SEARCH_HPP__

#include "oaz/mcts/search.hpp"

namespace oaz::mcts {

	template <class Game>
	using AZSearch = Search<Game, AZSelector<SearchNode<typename Game::Move>>>;
}

#endif
