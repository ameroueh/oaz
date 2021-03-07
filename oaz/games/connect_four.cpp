#include "oaz/games/connect_four.hpp"

#include <algorithm>
#include <bitset>
#include <string>
#include <vector>

using namespace oaz::games;

ConnectFour::ConnectFour() : m_status(0) {}

void ConnectFour::Reset() { *this = ConnectFour(); }

void ConnectFour::PlayFromString(std::string moves) {
  for (char& c : moves) PlayMove(c - '0');
}

size_t ConnectFour::GetCurrentPlayer() const {
  return m_player0_tokens.Sum() == m_player1_tokens.Sum() ? 0 : 1;
}

size_t ConnectFour::GetNumberOfTokensInColumn(size_t column) const {
  return (m_player0_tokens | m_player1_tokens).ColumnSum(column);
}

const ConnectFour::Board& ConnectFour::GetPlayerBoard(size_t player) const {
  return (player == 0) ? m_player0_tokens : m_player1_tokens;
}

ConnectFour::Board& ConnectFour::GetPlayerBoard(size_t player) {
  return const_cast<Board&>(
      static_cast<const ConnectFour&>(*this).GetPlayerBoard(player));
}

void ConnectFour::PlayMove(size_t move) {
  size_t player = GetCurrentPlayer();
  size_t n_tokens_in_column =
      (m_player0_tokens | m_player1_tokens).ColumnSum(move);
  Board& board = GetPlayerBoard(player);
  board.Set(n_tokens_in_column, move);
  bool victory = CheckVictory(board, n_tokens_in_column, move);
  MaybeEndGame(victory, player);
}

void ConnectFour::MaybeEndGame(bool victory, size_t player) {
  if (victory) {
    SetWinner(player);
    DeclareFinished();
  } else if ((m_player0_tokens | m_player1_tokens).Sum() == 42)
    DeclareFinished();
}

void ConnectFour::DeclareFinished() { m_status.set(2); }

void ConnectFour::GetAvailableMoves(std::vector<size_t>& moves) const {
  moves.clear();

  if (IsFinished()) return;

  Board board = m_player0_tokens | m_player1_tokens;
  for (size_t i = 0; i != 7; ++i)
    if (board.ColumnSum(i) < 6) moves.push_back(i);
}

bool ConnectFour::IsFinished() const { return m_status.test(2); }

void ConnectFour::SetWinner(size_t player) { m_status.set(player); }

bool ConnectFour::CheckVictory(const Board& board, size_t row,
                               size_t column) const {
  return CheckVerticalVictory(board, row, column) ||
         CheckHorizontalVictory(board, row, column) ||
         CheckDiagonalVictory1(board, row, column) ||
         CheckDiagonalVictory2(board, row, column);
}

bool ConnectFour::CheckVictory(const Board& board) const {
  bool victory = false;
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      if (board.Get(i, j)) {
        victory = CheckVictory(board, i, j);
        if (victory) return victory;
      }
  return victory;
}

void ConnectFour::CheckVictory() {
  const Board& player0_tokens = GetPlayerBoard(0);
  bool victory0 = CheckVictory(player0_tokens);
  MaybeEndGame(victory0, 0);
  const Board& player1_tokens = GetPlayerBoard(1);
  bool victory1 = CheckVictory(player1_tokens);
  MaybeEndGame(victory1, 1);
}

bool ConnectFour::CheckVerticalVictory(const ConnectFour::Board& board,
                                       size_t row, size_t column) const {
  Board mask = COLUMN;
  mask.PositiveColumnShift(column);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

bool ConnectFour::CheckHorizontalVictory(const ConnectFour::Board& board,
                                         size_t row, size_t column) const {
  Board mask = ROW;
  mask.PositiveRowShift(row);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

bool ConnectFour::CheckDiagonalVictory1(const ConnectFour::Board& board,
                                        size_t row, size_t column) const {
  Board mask = FIRST_DIAGONAL;
  if (row > column)
    mask.NegativeColumnShift(row - column);
  else if (column > row)
    mask.PositiveColumnShift(column - row);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

bool ConnectFour::CheckDiagonalVictory2(const ConnectFour::Board& board,
                                        size_t row, size_t column) const {
  Board mask = SECOND_DIAGONAL;
  size_t column_match = 5 - row;
  if (column > column_match)
    mask.PositiveColumnShift(column - column_match);
  else if (column < column_match)
    mask.NegativeColumnShift(column_match - column);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

bool ConnectFour::Player0Won() const { return m_status.test(0); }

bool ConnectFour::Player1Won() const { return m_status.test(1); }

float ConnectFour::GetScore() const {
  if (IsFinished())
    if (Player0Won())
      return 1;
    else if (Player1Won())
      return -1;
    else
      return 0;
  else
    return 0;
}

std::unique_ptr<Game> ConnectFour::Clone() const {
  return std::make_unique<ConnectFour>(*this);
}

bool ConnectFour::operator==(const ConnectFour& rhs) const {
  return m_player0_tokens == rhs.m_player0_tokens &&
         m_player1_tokens == rhs.m_player1_tokens && m_status == rhs.m_status;
}

void ConnectFour::WriteStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[6][7][2]);
  const Board& player0_tokens = GetPlayerBoard(0);
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      tensor[i][j][0] = player0_tokens.Get(i, j) ? 1. : 0.;
  const Board& player1_tokens = GetPlayerBoard(1);
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      tensor[i][j][1] = player1_tokens.Get(i, j) ? 1. : 0.;
}

void ConnectFour::WriteCanonicalStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[6][7][2]);
  const Board& current_player_tokens = GetPlayerBoard(GetCurrentPlayer());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      tensor[i][j][0] = current_player_tokens.Get(i, j) ? 1. : 0.;
  const Board& other_player_tokens = GetPlayerBoard(1 - GetCurrentPlayer());
  for (size_t i = 0; i != 6; ++i)
    for (size_t j = 0; j != 7; ++j)
      tensor[i][j][1] = other_player_tokens.Get(i, j) ? 1. : 0.;
}

void ConnectFour::InitialiseFromState(float* input_board) {
  Reset();
  size_t player_0 = 0;
  size_t player_1 = 1;

  Board& player0_tokens = GetPlayerBoard(player_0);
  Board& player1_tokens = GetPlayerBoard(player_1);

  boost::multi_array_ref<float, 3> data(input_board, boost::extents[6][7][2]);

  for (int i = 0; i != 6; ++i) {
    for (int j = 0; j != 7; ++j) {
      if (data[i][j][0] == 1.0f)
        player0_tokens.Set(i, j);
      else if (data[i][j][1] == 1.0f)
        player1_tokens.Set(i, j);
    }
  }
  CheckVictory();
}

void ConnectFour::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 3> data(input_board, boost::extents[6][7][2]);

  float current_player_count = 0.0;
  float other_player_count = 0.0;
  for (size_t i = 0; i != 6; ++i) {
    for (size_t j = 0; j != 7; ++j) {
      current_player_count += data[i][j][0];
      other_player_count += data[i][j][1];
    }
  }

  int current_player = current_player_count == other_player_count ? 0 : 1;
  int other_player = 1 - current_player;

  Board& current_player_tokens = GetPlayerBoard(current_player);
  Board& other_player_tokens = GetPlayerBoard(other_player);

  for (size_t i = 0; i != 6; ++i) {
    for (size_t j = 0; j != 7; ++j) {
      if (data[i][j][0] == 1.0f)
        current_player_tokens.Set(i, j);
      else if (data[i][j][1] == 1.0f)
        other_player_tokens.Set(i, j);
    }
  }
  CheckVictory();
}

uint64_t ConnectFour::GetState() const {
  return m_player0_tokens.GetBits() | (m_player1_tokens.ColumnSum(0) << 42) |
         (m_player1_tokens.ColumnSum(1) << 45) |
         (m_player1_tokens.ColumnSum(2) << 48) |
         (m_player1_tokens.ColumnSum(3) << 51) |
         (m_player1_tokens.ColumnSum(4) << 54) |
         (m_player1_tokens.ColumnSum(5) << 57) |
         (m_player1_tokens.ColumnSum(6) << 60);
}