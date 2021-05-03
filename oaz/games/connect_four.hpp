#ifndef OAZ_GAMES_CONNECT_FOUR_HPP_
#define OAZ_GAMES_CONNECT_FOUR_HPP_

#include <stdint.h>

#include <bitset>
#include <memory>
#include <string>
#include <vector>

#include "oaz/array/array.hpp"
#include "oaz/bitboard/bitboard.hpp"
#include "oaz/games/game.hpp"
#include "oaz/games/generic_game_map.hpp"

namespace oaz::games {
class ConnectFour : public Game {
 public:
  struct Class : public Game::Class {
    size_t GetMaxNumberOfMoves() const override { return N_COLUMNS; }
    const std::vector<int>& GetBoardShape() const override { return m_board_shape; }
    GameMap* CreateGameMap() const override {
      return new GenericGameMap<ConnectFour, uint64_t>();
    }
    static const Class& Methods() {
      static const Class meta;
      return meta;
    }

   private:
    const std::vector<int> m_board_shape{N_ROWS, N_COLUMNS, N_PLAYERS};
  };
  const Class& ClassMethods() const override { return Class::Methods(); }

  ConnectFour();

  void PlayFromString(std::string) override;
  void PlayMove(size_t) override;
  size_t GetCurrentPlayer() const override;
  bool IsFinished() const override;
  void GetAvailableMoves(std::vector<size_t>*) const override;
  float GetScore() const override;
  void WriteStateToTensorMemory(float*) const override;
  void WriteCanonicalStateToTensorMemory(float*) const override;
  void InitialiseFromState(float*) override;
  void InitialiseFromCanonicalState(float*) override;
  std::unique_ptr<Game> Clone() const override;

  bool operator==(const ConnectFour&) const;

  uint64_t GetState() const;

 private:

  static constexpr size_t N_COLUMNS = 7;
  static constexpr size_t N_ROWS = 6;
  static constexpr size_t N_PLAYERS = 2;
  static constexpr size_t N_STATUS_BITS = 8;
  static constexpr size_t N_SQUARES = N_ROWS * N_COLUMNS;

  using Board = oaz::bitboard::BitBoard<N_ROWS, N_COLUMNS>;
  static constexpr Board ROW{{0, 0}, {0, 1}, {0, 2}, {0, 3},
                             {0, 4}, {0, 5}, {0, 6}};
  static constexpr Board COLUMN{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};
  static constexpr Board FIRST_DIAGONAL{{0, 0}, {1, 1}, {2, 2},
                                        {3, 3}, {4, 4}, {5, 5}};
  static constexpr Board SECOND_DIAGONAL{{5, 0}, {4, 1}, {3, 2},
                                         {2, 3}, {1, 4}, {0, 5}};

  size_t GetNumberOfTokensInColumn(size_t) const;
  template <class G>
  static auto& GetPlayerBoardImpl(G&, size_t);
  Board& GetPlayerBoard(size_t);
  const Board& GetPlayerBoard(size_t) const;

  static inline bool CheckVictory(const Board&, size_t, size_t);
  static inline bool CheckVictory(const Board&);

  static inline bool CheckVerticalVictory(const Board&, size_t, size_t);
  static inline bool CheckHorizontalVictory(const Board&, size_t, size_t);
  static inline bool CheckDiagonalVictory1(const Board&, size_t, size_t);
  static inline bool CheckDiagonalVictory2(const Board&, size_t, size_t);
  bool Player0Won() const;
  bool Player1Won() const;

  void CheckVictory();
  void MaybeEndGame(bool, size_t);
  void SetWinner(size_t);
  void DeclareFinished();
  void Reset();

  Board m_player0_tokens;
  Board m_player1_tokens;
  std::bitset<N_STATUS_BITS> m_status;
};
}  // namespace oaz::games
#endif  // OAZ_GAMES_CONNECT_FOUR_HPP_
