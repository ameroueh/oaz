#ifndef OAZ_GAMES_BANDITS_HPP_
#define OAZ_GAMES_BANDITS_HPP_

#include <stdint.h>

#include <bitset>
#include <memory>
#include <string>
#include <vector>

#include "oaz/array/array.hpp"
#include "oaz/games/game.hpp"
#include "oaz/games/generic_game_map.hpp"

namespace oaz::games {
class Bandits : public Game {
 public:
  struct Class : public Game::Class {
    size_t GetMaxNumberOfMoves() const override { return N_ROWS; }
    const std::vector<int>& GetBoardShape() const override { return m_board_shape; }
    static const Class& Methods() {
      static const Class meta;
      return meta;
    }
    GameMap* CreateGameMap() const override {
      return new GenericGameMap<Bandits, uint64_t>();
    }

   private:
    const std::vector<int> m_board_shape{N_ROWS};
  };
  const Class& ClassMethods() const override { return Class::Methods(); }

  Bandits();

  void PlayFromString(std::string moves) override;
  void PlayMove(size_t move) override;
  size_t GetCurrentPlayer() const override;
  bool IsFinished() const override;
  void GetAvailableMoves(std::vector<size_t>* available_moves) const override;
  float GetScore() const override;
  void WriteStateToTensorMemory(float* destination) const override;
  void WriteCanonicalStateToTensorMemory(float* destination) const override;
  void InitialiseFromCanonicalState(float* input_board) override;
  void InitialiseFromState(float* input_board) override;
  std::unique_ptr<Game> Clone() const override;

  bool operator==(const Bandits&) const;

  uint64_t GetState() const;

 private:
  static constexpr size_t N_ROWS = 10;
  static constexpr std::bitset<10> WINNING_BITS =
      std::bitset<N_ROWS>(0b101010101ULL);
  std::bitset<N_ROWS> m_board;
  void Reset();
};
}  // namespace oaz::games
#endif  // OAZ_GAMES_BANDITS_HPP_
