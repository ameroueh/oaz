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
    size_t GetMaxNumberOfMoves() const override { return N_SQUARES; }
    const std::vector<int>& GetBoardShape() const override {
      return m_board_shape;
    }
    GameMap* CreateGameMap() const override {
      return new GenericGameMap<TicTacToe, uint64_t>();
    }
    static const Class& Methods() {
      static const Class meta;
      return meta;
    }

   private:
    const std::vector<int> m_board_shape{SIDE_LENGTH, SIDE_LENGTH, N_PLAYERS};
  };
  const Class& ClassMethods() const override { return Class::Methods(); }

  TicTacToe();

  void PlayFromString(std::string moves) override;
  void PlayMove(size_t move) override;
  void GetAvailableMoves(std::vector<size_t>* available_moves) const override;
  size_t GetCurrentPlayer() const override;
  bool IsFinished() const override;
  float GetScore() const override;
  void WriteStateToTensorMemory(float* destination) const override;
  void WriteCanonicalStateToTensorMemory(float* destination) const override;
  void InitialiseFromState(float* input_board) override;
  void InitialiseFromCanonicalState(float* input_board) override;
  std::unique_ptr<Game> Clone() const override;

  bool operator==(const TicTacToe&);

  size_t GetState() const;

 private:
  static constexpr size_t SIDE_LENGTH = 3;
  static constexpr size_t N_SQUARES = 9;
  static constexpr size_t N_PLAYERS = 2;
  static constexpr size_t N_STATUS_BITS = 8;

  using Board = oaz::bitboard::BitBoard<SIDE_LENGTH, SIDE_LENGTH>;
  static constexpr Board FIRST_DIAGONAL{{0, 0}, {1, 1}, {2, 2}};
  static constexpr Board SECOND_DIAGONAL{{2, 0}, {1, 1}, {0, 2}};

  template <class G>
  static auto& GetPlayerBoardImpl(G&, size_t);
  const Board& GetPlayerBoard(size_t) const;
  Board& GetPlayerBoard(size_t);
  static inline bool CheckVictory(const Board&, size_t, size_t);
  static inline bool CheckVictory(const Board&);
  inline void CheckVictory();
  void MaybeEndGame(bool, size_t);
  void SetWinner(size_t);
  void DeclareFinished();
  void Reset();
  bool Player0Won() const;
  bool Player1Won() const;

  Board m_player0_tokens;
  Board m_player1_tokens;
  std::bitset<N_STATUS_BITS> m_status;
};
}  // namespace oaz::games
#endif  // OAZ_GAMES_TIC_TAC_TOE_HPP_
