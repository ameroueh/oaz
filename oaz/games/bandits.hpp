#ifndef __BANDITS_HPP__
#define __BANDITS_HPP__

#include <vector>
#include <string>

#include "stdint.h"
#include "oaz/array/array.hpp"


namespace oaz::games {
	class Bandits {
		public:
			using Move = uint32_t;
			using Value = float;
			using Policy = oaz::array::Array<10>;
			using Tile = float;
			using Board = oaz::array::Array<10>;
			
			static constexpr Tile EMPTY_TOKEN = 0.;
			static constexpr Tile BASE_TOKEN = 1.;
			
			static const size_t n_moves = 10;
			static const size_t n_players = 2;
			static const size_t max_n_moves = 10;

			Bandits();
			Bandits(const Bandits&);
			
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

			bool operator==(const Bandits&);
			void set(const Bandits&);
				
		private:
			void setCurrentPlayer(size_t);
			void initialise();
			void resetBoard();
			void placeToken(Move);
			void removeToken(Move);
			void swapPlayers();
			void refreshAvailableMoves();
			void maybeDeclareVictory(Move);

			size_t m_current_player;
			float m_score;
			bool m_game_won;
			std::vector<Move> m_available_moves;
			
			Board m_board;
	};
}

#include "oaz/games/bandits_impl.cpp"
#endif
