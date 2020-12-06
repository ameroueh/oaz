#ifndef __MCTS_SEARCH_HPP__
#define __MCTS_SEARCH_HPP__

#include "oaz/mcts/search.hpp"

namespace oaz::mcts {

	using MCTSSearch = Search<UCTSelector<SearchNode>>;
}

#endif
