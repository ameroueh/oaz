#ifndef __TIC_TAC_TOE_HPP__
#define __TIC_TAC_TOE_HPP__

#include <vector>
#include <string>

#include "stdint.h"
#include "oaz/array/array.hpp"


namespace oaz::games {
	class TicTacToe {
		public:
			using Move = uint32_t;
			using Value = float;
			using Policy = oaz::array::Array<9>;
			using Tile = float;
			using Board = oaz::array::Array<3, 3, 2>;
			
			static constexpr Tile EMPTY_TOKEN = 0.;
			static constexpr Tile BASE_TOKEN = 1.;
			
			static const size_t n_moves = 9;
			static const size_t n_players = 2;
			static const size_t max_n_moves = 9;

			TicTacToe();
			TicTacToe(const TicTacToe&);
			
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

			bool operator==(const TicTacToe&);
			void set(const TicTacToe&);
				
		private:
			void setCurrentPlayer(size_t);
			void initialise();
			void resetBoard();
			void placeToken(Move);
			void removeToken(Move);
			void swapPlayers();
			void refreshAvailableMoves();
			void maybeDeclareVictory(Move);

			bool checkVerticalVictory(size_t, size_t);
			bool checkHorizontalVictory(size_t, size_t);
			bool checkFirstDiagonalVictory(size_t);
			bool checkSecondDiagonalVictory(size_t);

			size_t m_current_player;
			float m_score;
			bool m_game_won;
			std::vector<Move> m_available_moves;
			
			Board m_board;
	};
}

#include "oaz/games/tic_tac_toe_impl.cpp"
#endif
