#ifndef __GAME_HPP__
#define __GAME_HPP__

#include <memory>
#include <string>


namespace oaz::games {

	class Game {
		public:
			struct Class {
				virtual size_t GetMaxNumberOfMoves() const = 0;
				virtual const std::vector<int>& GetBoardShape() const = 0;

			};
			virtual const Class& ClassMethods() const = 0;
			

			virtual void PlayFromString(std::string) = 0;
			virtual void PlayMove(size_t) = 0;
			virtual void GetAvailableMoves(std::vector<size_t>&) const = 0;
			virtual float GetScore() const = 0;
			virtual size_t GetCurrentPlayer() const = 0;
			virtual bool IsFinished() const = 0;
			virtual void WriteStateToTensorMemory(float*) const = 0;
			virtual std::unique_ptr<Game> Clone() const = 0;

			virtual ~Game(){};

	};
};
#endif
