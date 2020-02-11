#include <iostream>

#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"

using namespace std;
using namespace oaz::mcts;
using namespace oaz::games;

using Board = ArrayBoard3D<float, 7, 6, 2>;
using Game = ConnectFour<Board>;
using Move = typename ConnectFour<Board>::move_t;
using Node = SearchNode<Game::move_t>;
using GamesContainer = vector<Game>;
using Evaluator = RandomEvaluator<Game, GamesContainer>;
using GameSearch = Search<Game, Evaluator>;

int main() {
	Board board;
	GamesContainer games(1, Game(board));
	Evaluator evaluator(&games);

	Board board2;
	Game game(board2);

	GameSearch search (1, game, &evaluator); 

	search.search(10000);
	
	Node* root = search.getTreeRoot();		
	std::cout << serialiseTreeToJson<Move>(root) << std::endl;
}
