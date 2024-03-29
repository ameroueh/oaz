#ifndef __GAME_HPP__
#define __GAME_HPP__

#include <memory>
#include <string>
#include <vector>

namespace oaz::games {

class Game {
 public:
  class GameMap {
   public:
    virtual bool Get(const Game&, size_t*) const = 0;
    virtual void Insert(const Game&, size_t) = 0;
    virtual size_t GetSize() const = 0;

    virtual ~GameMap() {}
    GameMap() = default;
    GameMap(const GameMap&) = default;
    GameMap(GameMap&&) = default;
    GameMap& operator=(const GameMap&) = default;
    GameMap& operator=(GameMap&&) = default;
  };

  struct Class {
    virtual size_t GetMaxNumberOfMoves() const = 0;
    virtual const std::vector<int>& GetBoardShape() const = 0;
    virtual GameMap* CreateGameMap() const = 0;
  };

  virtual const Class& ClassMethods() const = 0;

  virtual void PlayFromString(std::string) = 0;
  virtual void PlayMove(size_t) = 0;
  virtual void GetAvailableMoves(std::vector<size_t>*) const = 0;
  virtual float GetScore() const = 0;
  virtual size_t GetCurrentPlayer() const = 0;
  virtual bool IsFinished() const = 0;
  virtual void WriteStateToTensorMemory(float*) const = 0;
  virtual void WriteCanonicalStateToTensorMemory(float*) const = 0;
  virtual void InitialiseFromState(float*) = 0;
  virtual void InitialiseFromCanonicalState(float*) = 0;
  virtual std::unique_ptr<Game> Clone() const = 0;

  virtual ~Game(){};
  Game() = default;
  Game(const Game&) = default;
  Game(Game&&) = default;
  Game& operator=(const Game&) = default;
  Game& operator=(Game&&) = default;
};
};  // namespace oaz::games
#endif
