#ifndef OAZ_GAMES_TIC_TAC_TOE_HPP_
#define OAZ_GAMES_TIC_TAC_TOE_HPP_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "oaz/array/array.hpp"
#include "oaz/bitboard/bitboard.hpp"
#include "oaz/games/game.hpp"
#include "oaz/games/generic_game_map.hpp"

namespace oaz::games {
class TicTacToe : public Game {
 public:
  struct Class : public Game::Class {
    size_t GetMaxNumberOfMoves() const { return 9; }
    const std::vector<int>& GetBoardShape() const { return m_board_shape; }
    GameMap* CreateGameMap() const {
      return new GenericGameMap<TicTacToe, uint64_t>();
    }
    static const Class& Methods() {
      static const Class meta;
      return meta;
    }

   private:
    const std::vector<int> m_board_shape{3, 3, 2};
  };
  const Class& ClassMethods() const { return Class::Methods(); }

  TicTacToe();

  void PlayFromString(std::string);
  void PlayMove(size_t);
  void GetAvailableMoves(std::vector<size_t>&) const;
  size_t GetCurrentPlayer() const;
  bool IsFinished() const;
  float GetScore() const;
  void WriteStateToTensorMemory(float*) const;
  void WriteCanonicalStateToTensorMemory(float*) const;
  void InitialiseFromState(float*);
  void InitialiseFromCanonicalState(float*);
  std::unique_ptr<Game> Clone() const;

  bool operator==(const TicTacToe&);

  size_t GetState() const;

 private:
  using Board = oaz::bitboard::BitBoard<3, 3>;
  static constexpr Board FIRST_DIAGONAL{{0, 0}, {1, 1}, {2, 2}};
  static constexpr Board SECOND_DIAGONAL{{2, 0}, {1, 1}, {0, 2}};

  const Board& GetPlayerBoard(size_t) const;
  Board& GetPlayerBoard(size_t);
  bool CheckVictory(const Board&, size_t, size_t) const;
  bool CheckVictory(const Board&) const;
  void CheckVictory();
  void MaybeEndGame(bool, size_t);
  void SetWinner(size_t);
  void DeclareFinished();
  void Reset();
  bool Player0Won() const;
  bool Player1Won() const;

  Board m_player0_tokens;
  Board m_player1_tokens;
  std::bitset<8> m_status;
};
}  // namespace oaz::games
#endif // OAZ_GAMES_TIC_TAC_TOE_HPP_
