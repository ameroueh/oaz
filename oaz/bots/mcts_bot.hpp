#include <iostream>
#include <string>

#include "oaz/games/board.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/random/random_evaluator.hpp"
#include "oaz/mcts/search.hpp"
#include "oaz/mcts/search_node.hpp" 
#include "oaz/mcts/selection.hpp"
#include "oaz/mcts/search_node_serialisation.hpp"

namespace oaz::bots {
	class ConnectFourMCTSBot {
		public:
			using Board = ArrayBoard3D<float, 7, 6, 2>;
			using Game = ConnectFour<Board>;
			using Move = typename ConnectFour<Board>::move_t;
			using Node = SearchNode<Game::move_t>;
			using GamesContainer = vector<Game>;
			using Evaluator = RandomEvaluator<Game, GamesContainer>;
			using GameSearch = Search<Game, Evaluator>;

			MCTSBot(std::string configuration) {
					
			}
			
			void setPosition(std::string);
			void think();
			void getBestMove();
		private:
			Board m_board;
			Game m_game;
			GamesContainer m_games;



		
	};
}
