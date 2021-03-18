#include "oaz/games/tic_tac_toe.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

oaz::games::TicTacToe::TicTacToe() : m_status(0) {}

void oaz::games::TicTacToe::Reset() { *this = TicTacToe(); }

void oaz::games::TicTacToe::PlayFromString(std::string moves) {
  for (char& c : moves) {
    PlayMove(c - '0');
  }
}

void oaz::games::TicTacToe::PlayMove(size_t move) {
  size_t row = move % 3;
  size_t column = move / 3;

  size_t player = GetCurrentPlayer();
  Board& board = GetPlayerBoard(player);
  board.Set(row, column);
  bool victory = CheckVictory(board, row, column);
  MaybeEndGame(victory, player);
}

void oaz::games::TicTacToe::MaybeEndGame(bool victory, size_t player) {
  if (victory) {
    SetWinner(player);
    DeclareFinished();
  } else {
    if ((m_player0_tokens | m_player1_tokens).Sum() == 9) DeclareFinished();
  }
}

size_t oaz::games::TicTacToe::GetCurrentPlayer() const {
  return m_player0_tokens.Sum() == m_player1_tokens.Sum() ? 0 : 1;
}

const oaz::games::TicTacToe::Board& oaz::games::TicTacToe::GetPlayerBoard(
    size_t player) const {
  return (player == 0) ? m_player0_tokens : m_player1_tokens;
}

oaz::games::TicTacToe::Board& oaz::games::TicTacToe::GetPlayerBoard(
    size_t player) {
  return const_cast<Board&>(
      static_cast<const TicTacToe&>(*this).GetPlayerBoard(player));
}

bool oaz::games::TicTacToe::CheckVictory(const Board& board, size_t row,
                                         size_t column) const {
  if ((board.RowSum(row) == 3) || (board.ColumnSum(column) == 3)) {
    return true;
  }
  if ((row == column) && ((board & FIRST_DIAGONAL).Sum() == 3)) {
    return true;
  }
  if ((row == 2 - column) && ((board & SECOND_DIAGONAL).Sum() == 3)) {
    return true;
  }
  return false;
}

bool oaz::games::TicTacToe::CheckVictory(const Board& board) const {
  bool victory = false;
  for (size_t i = 0; i != 3; ++i)
    for (size_t j = 0; j != 3; ++j)
      if (board.Get(i, j)) {
        victory = CheckVictory(board, i, j);
        if (victory) return victory;
      }
  return victory;
}

void oaz::games::TicTacToe::CheckVictory() {
  const Board& player0_tokens = GetPlayerBoard(0);
  bool victory0 = CheckVictory(player0_tokens);
  MaybeEndGame(victory0, 0);
  const Board& player1_tokens = GetPlayerBoard(1);
  bool victory1 = CheckVictory(player1_tokens);
  MaybeEndGame(victory1, 1);
}

void oaz::games::TicTacToe::SetWinner(size_t player) { m_status.set(player); }

void oaz::games::TicTacToe::DeclareFinished() { m_status.set(2); }

void oaz::games::TicTacToe::GetAvailableMoves(
    std::vector<size_t>& moves) const {
  moves.clear();

  if (IsFinished()) return;

  Board board = m_player0_tokens | m_player1_tokens;
  for (size_t move = 0; move != 9; ++move) {
    size_t row = move % 3;
    size_t column = move / 3;
    if (!board.Get(row, column)) moves.push_back(move);
  }
}

bool oaz::games::TicTacToe::IsFinished() const { return m_status.test(2); }

bool oaz::games::TicTacToe::Player0Won() const { return m_status.test(0); }

bool oaz::games::TicTacToe::Player1Won() const { return m_status.test(1); }

float oaz::games::TicTacToe::GetScore() const {
  if (IsFinished()) {
    if (Player0Won())
      return 1;
    else if (Player1Won())
      return -1;
    else
      return 0;
  } else {
    return 0;
  }
}

bool oaz::games::TicTacToe::operator==(const TicTacToe& rhs) {
  return (m_player0_tokens == rhs.m_player0_tokens) &&
         (m_player1_tokens == rhs.m_player1_tokens) &&
         (m_status == rhs.m_status);
}

std::unique_ptr<oaz::games::Game> oaz::games::TicTacToe::Clone() const {
  return std::make_unique<TicTacToe>(*this);
}

void oaz::games::TicTacToe::WriteStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[3][3][2]);
  const Board& player0_tokens = GetPlayerBoard(0);
  for (size_t i = 0; i != 3; ++i)
    for (size_t j = 0; j != 3; ++j)
      tensor[i][j][0] = player0_tokens.Get(i, j) ? 1. : 0.;
  const Board& player1_tokens = GetPlayerBoard(1);
  for (size_t i = 0; i != 3; ++i)
    for (size_t j = 0; j != 3; ++j)
      tensor[i][j][1] = player1_tokens.Get(i, j) ? 1. : 0.;
}

void oaz::games::TicTacToe::WriteCanonicalStateToTensorMemory(
    float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[3][3][2]);
  const Board& current_player_tokens = GetPlayerBoard(GetCurrentPlayer());
  for (size_t i = 0; i != 3; ++i)
    for (size_t j = 0; j != 3; ++j)
      tensor[i][j][0] = current_player_tokens.Get(i, j) ? 1. : 0.;
  const Board& other_player_tokens = GetPlayerBoard(1 - GetCurrentPlayer());
  for (size_t i = 0; i != 3; ++i)
    for (size_t j = 0; j != 3; ++j)
      tensor[i][j][1] = other_player_tokens.Get(i, j) ? 1. : 0.;
}

void oaz::games::TicTacToe::InitialiseFromState(float* input_board) {
  Reset();
  size_t player_0 = 0;
  size_t player_1 = 1;

  Board& player0_tokens = GetPlayerBoard(player_0);
  Board& player1_tokens = GetPlayerBoard(player_1);

  boost::multi_array_ref<float, 3> data(input_board, boost::extents[3][3][2]);

  for (size_t i = 0; i != 3; ++i) {
    for (size_t j = 0; j != 3; ++j) {
      if (data[i][j][0] == 1.0f)
        player0_tokens.Set(i, j);
      else if (data[i][j][1] == 1.0f)
        player1_tokens.Set(i, j);
    }
  }
  CheckVictory();
}

void oaz::games::TicTacToe::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 3> data(input_board, boost::extents[3][3][2]);

  float current_player_count = 0.0;
  float other_player_count = 0.0;
  for (size_t i = 0; i != 3; ++i) {
    for (size_t j = 0; j != 3; ++j) {
      current_player_count += data[i][j][0];
      other_player_count += data[i][j][1];
    }
  }

  int current_player = current_player_count == other_player_count ? 0 : 1;
  int other_player = 1 - current_player;

  Board& current_player_tokens = GetPlayerBoard(current_player);
  Board& other_player_tokens = GetPlayerBoard(other_player);

  for (size_t i = 0; i != 3; ++i) {
    for (size_t j = 0; j != 3; ++j) {
      if (data[i][j][0] == 1.0f)
        current_player_tokens.Set(i, j);
      else if (data[i][j][1] == 1.0f)
        other_player_tokens.Set(i, j);
    }
  }
  CheckVictory();
}

size_t oaz::games::TicTacToe::GetState() const {
  return m_player0_tokens.GetBits() | (m_player1_tokens.GetBits() << 9);
}
