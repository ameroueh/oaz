#ifndef __CONNECT_FOUR_HPP__
#define __CONNECT_FOUR_HPP__

#include <vector>

#include "stdint.h"
#include "boost/multi_array.hpp"


namespace oaz {
	namespace games {
		template <class Board>
		class ConnectFour {
			public:
				using gsize_t = int32_t;
				using move_t = uint32_t;
				using tile_t = float;

				using boardptr_t = Board*;

				using tregistry_t = typename boost::multi_array<gsize_t, 1>;

				static const gsize_t width = 7;
				static const gsize_t height = 6;
				static const gsize_t n_moves = 7;
				static const gsize_t n_players = 2;

				static constexpr tile_t EMPTY_TOKEN = 0.;
				static constexpr tile_t BASE_TOKEN = 1.;
				ConnectFour(boardptr_t);
				
				void reset();

				void playMove(move_t);
				void undoMove(move_t);
				bool Finished() const;
				std::vector<move_t>* availableMoves();
				float score() const;

				gsize_t currentPlayer() const;

				bool operator==(const ConnectFour&);
					
			private:
				void initialise();
				void resetBoard();
				void placeToken(move_t);
				void removeToken(move_t);
				void swapPlayers();
				void refreshAvailableMoves();
				void maybeDeclareVictory(move_t);

				bool checkVerticalVictory(gsize_t, gsize_t, gsize_t);
				bool checkHorizontalVictory(gsize_t, gsize_t, gsize_t);
				bool checkFirstDiagonalVictory(gsize_t, gsize_t, gsize_t);
				bool checkSecondDiagonalVictory(gsize_t, gsize_t, gsize_t);

				gsize_t m_current_player;
				float m_score;
				bool m_game_won;
				std::vector<move_t> m_available_moves;
				boardptr_t m_board;
				tregistry_t m_tokens_in_column;
		};
	}
}

#include "oaz/games/connect_four_impl.cpp"
#endif

