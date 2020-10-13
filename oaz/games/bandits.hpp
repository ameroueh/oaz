#ifndef __BANDITS_HPP__
#define __BANDITS_HPP__

#include <bitset>
#include <memory>
#include <vector>
#include <string>

#include "oaz/array/array.hpp"
#include "oaz/games/game.hpp"

namespace oaz::games {
	class Bandits : public Game {
		public:
			struct Class : public Game::Class {
				size_t GetMaxNumberOfMoves() const {
					return 10;
				}
				const std::vector<int>& GetBoardShape() const {
					return m_board_shape;
				}
				static const Class& Methods() {
					static const Class meta;
					return meta;
				};

				private:
					const std::vector<int> m_board_shape {10};
			};
			const Class& ClassMethods() const {
				return Class::Methods();
			}
			
			Bandits();
			
			void PlayFromString(std::string);
			void PlayMove(size_t);
			size_t GetCurrentPlayer() const;
			bool IsFinished() const;
			void GetAvailableMoves(std::vector<size_t>&) const;
			float GetScore() const;
			void WriteStateToTensorMemory(float*) const;
			std::unique_ptr<Game> Clone() const;

			bool operator==(const Bandits&) const;	
		private:
			static constexpr std::bitset<10> WINNING_BITS = std::bitset<10>(0b101010101ll);
			std::bitset<10> m_board;
	};
}

#include "oaz/games/bandits_impl.cpp"
#endif
