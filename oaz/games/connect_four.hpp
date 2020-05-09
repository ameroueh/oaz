#ifndef __CONNECT_FOUR_HPP__
#define __CONNECT_FOUR_HPP__

#include <vector>

#include "stdint.h"
#include "boost/multi_array.hpp"


namespace oaz::games {
	class ConnectFour {
		public:
			using Move = uint32_t;
			using Value = float;
			using Policy = std::array<float, 7>;
			using Tile = float;
			using Board = boost::multi_array<float, 3>;
			using Registry = std::array<size_t, 7>;
			
			static constexpr Tile EMPTY_TOKEN = 0.;
			static constexpr Tile BASE_TOKEN = 1.;
			
			static const size_t width = 7;
			static const size_t height = 6;
			static const size_t n_moves = 7;
			static const size_t n_players = 2;
			static const size_t max_n_moves = 42;
			static const size_t NBoardDimensions = 3;

			static std::vector<long long int> getBoardDimensions() {return {7, 6, 2};}
			static std::vector<unsigned long long> getBoardDimensionsUnsigned() {return {7, 6, 2};}
			static size_t getPolicySize() {
				return 7;
			}

			ConnectFour();
			ConnectFour(const ConnectFour&);
			
			void reset();
			Board& getBoard() {
				return m_board;
			}

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
