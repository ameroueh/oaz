#ifndef __CONNECT_FOUR_HPP__
#define __CONNECT_FOUR_HPP__

#include <vector>

#include "stdint.h"
#include "boost/multi_array.hpp"


namespace oaz {
	namespace games {
		class ConnectFour {
			public:
				typedef int64_t gsize_t;
				typedef uint16_t tile_t;
				typedef uint16_t move_t;
				typedef boost::multi_array<tile_t, 3> board_t;
				typedef boost::multi_array<gsize_t, 1> tregistry_t;

				static const gsize_t width = 7;
				static const gsize_t height = 6;
				static const gsize_t n_moves = 7;
				static const gsize_t n_players = 2;

				ConnectFour();
				
				void reset();

				void playMove(move_t);
				void undoMove(move_t);
				bool Finished() const;
				std::vector<move_t>* availableMoves();
				gsize_t score() const;

				tile_t currentPlayer() const;

				bool operator==(const ConnectFour&);
					
			private:
				void initialise();
				void placeToken(move_t);
				void removeToken(move_t);
				void swapPlayers();
				void refreshAvailableMoves();
				void maybeDeclareVictory(move_t);

				bool checkVerticalVictory(gsize_t, gsize_t, tile_t);
				bool checkHorizontalVictory(gsize_t, gsize_t, tile_t);
				bool checkFirstDiagonalVictory(gsize_t, gsize_t, tile_t);
				bool checkSecondDiagonalVictory(gsize_t, gsize_t, tile_t);

				tile_t m_current_player;
				float m_score;
				bool m_game_won;
				std::vector<move_t> m_available_moves;
				board_t m_board;
				tregistry_t m_tokens_in_column;
		};
	}
}
#endif

