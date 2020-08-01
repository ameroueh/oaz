#ifndef __CONNECT_FOUR_HPP__
#define __CONNECT_FOUR_HPP__

#include <string>
#include <vector>

#include "stdint.h"
#include "oaz/array/array.hpp"


namespace oaz::games {
	class ConnectFour {
		public:
			using Move = uint32_t;
			using Value = float;
			using Tile = float;
			using Board = oaz::array::Array<7, 6, 2>;
			using Policy = oaz::array::Array<7>;
			using Registry = std::array<size_t, 7>;
			
			static constexpr Tile EMPTY_TOKEN = 0.;
			static constexpr Tile BASE_TOKEN = 1.;
			
			static const size_t n_moves = 7;
			static const size_t n_players = 2;
			static const size_t max_n_moves = 42;
			
			ConnectFour();
			ConnectFour(const ConnectFour&);
			
			void reset();
			Board& getBoard() {
				return m_board;
			}


			void playFromString(std::string);
			void playMove(Move);
			void undoMove(Move);
			size_t getCurrentPlayer() const;
			bool Finished() const;
			std::vector<Move>* availableMoves();
			float score() const;

			size_t currentPlayer() const;

			bool operator==(const ConnectFour&);
			void set(const ConnectFour&);
				
		private:
			void setCurrentPlayer(size_t);
			void initialise();
			void resetBoard();
			void placeToken(Move);
			void removeToken(Move);
			void swapPlayers();
			void refreshAvailableMoves();
			void maybeDeclareVictory(Move);

			bool checkVerticalVictory(size_t, size_t, size_t);
			bool checkHorizontalVictory(size_t, size_t, size_t);
			bool checkFirstDiagonalVictory(size_t, size_t, size_t);
			bool checkSecondDiagonalVictory(size_t, size_t, size_t);

			size_t m_current_player;
			float m_score;
			bool m_game_won;
			std::vector<Move> m_available_moves;
			
			Board m_board;
			Registry m_tokens_in_column;
	};
}

#include "oaz/games/connect_four_impl.cpp"
#endif
