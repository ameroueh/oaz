#include "oaz/games/connect_four.hpp"

#include <algorithm>
#include <bitset>
#include <memory>
#include <string>
#include <vector>

oaz::games::ConnectFour::ConnectFour() : m_status(0) {}

void oaz::games::ConnectFour::Reset() { *this = ConnectFour(); }

void oaz::games::ConnectFour::PlayFromString(std::string moves) {
  for (char& c : moves) {PlayMove(c - '0');}
}

size_t oaz::games::ConnectFour::GetCurrentPlayer() const {
  return m_player0_tokens.Sum() == m_player1_tokens.Sum() ? 0 : 1;
}

size_t oaz::games::ConnectFour::GetNumberOfTokensInColumn(size_t column) const {
  return (m_player0_tokens | m_player1_tokens).ColumnSum(column);
}

template <class G>
auto& oaz::games::ConnectFour::GetPlayerBoardImpl(G& game, size_t player) {
  return (player == 0) ? game.m_player0_tokens : game.m_player1_tokens;
}

const oaz::games::ConnectFour::Board& oaz::games::ConnectFour::GetPlayerBoard(
    size_t player) const {
  return GetPlayerBoardImpl(*this, player);
}

oaz::games::ConnectFour::Board& oaz::games::ConnectFour::GetPlayerBoard(
    size_t player) {
  return GetPlayerBoardImpl(*this, player);
}

void oaz::games::ConnectFour::PlayMove(size_t move) {
  size_t player = GetCurrentPlayer();
  size_t n_tokens_in_column =
      (m_player0_tokens | m_player1_tokens).ColumnSum(move);
  Board& board = GetPlayerBoard(player);
  board.Set(n_tokens_in_column, move);
  bool victory = CheckVictory(board, n_tokens_in_column, move);
  MaybeEndGame(victory, player);
}

void oaz::games::ConnectFour::MaybeEndGame(bool victory, size_t player) {
  if (victory) {
    SetWinner(player);
    DeclareFinished();
  } else if ((m_player0_tokens | m_player1_tokens).Sum() == N_SQUARES) {
    DeclareFinished();
  }
}

void oaz::games::ConnectFour::DeclareFinished() { m_status.set(N_PLAYERS); }

void oaz::games::ConnectFour::GetAvailableMoves(
    std::vector<size_t>* moves) const {
  moves->clear();

  if (IsFinished()) {return;}

  Board board = m_player0_tokens | m_player1_tokens;
  for (size_t i = 0; i != N_COLUMNS; ++i) {
    if (board.ColumnSum(i) < N_ROWS) {moves->push_back(i);}
  }
}

bool oaz::games::ConnectFour::IsFinished() const { return m_status.test(N_PLAYERS); }

void oaz::games::ConnectFour::SetWinner(size_t player) { m_status.set(player); }

inline bool oaz::games::ConnectFour::CheckVictory(const Board& board, size_t row,
                                           size_t column) {
  return CheckVerticalVictory(board, row, column) ||
         CheckHorizontalVictory(board, row, column) ||
         CheckDiagonalVictory1(board, row, column) ||
         CheckDiagonalVictory2(board, row, column);
}

inline bool oaz::games::ConnectFour::CheckVictory(const Board& board) {
  bool victory = false;
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      if (board.Get(i, j) == 1) {
        victory = CheckVictory(board, i, j);
        if (victory) {return victory;}
      }
    }
  }
  return victory;
}

inline void oaz::games::ConnectFour::CheckVictory() {
  const Board& player0_tokens = GetPlayerBoard(0);
  bool victory0 = CheckVictory(player0_tokens);
  MaybeEndGame(victory0, 0);
  const Board& player1_tokens = GetPlayerBoard(1);
  bool victory1 = CheckVictory(player1_tokens);
  MaybeEndGame(victory1, 1);
}

inline bool oaz::games::ConnectFour::CheckVerticalVictory(
    const oaz::games::ConnectFour::Board& board, size_t row,
    size_t column) {
  Board mask = COLUMN;
  mask.PositiveColumnShift(column);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

inline bool oaz::games::ConnectFour::CheckHorizontalVictory(
    const oaz::games::ConnectFour::Board& board, size_t row,
    size_t column) {
  Board mask = ROW;
  mask.PositiveRowShift(row);
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

inline bool oaz::games::ConnectFour::CheckDiagonalVictory1(
    const oaz::games::ConnectFour::Board& board, size_t row,
    size_t column) {
  Board mask = FIRST_DIAGONAL;
  if (row > column) {
    mask.NegativeColumnShift(row - column);
  } else {
    if (column > row) {
      mask.PositiveColumnShift(column - row);
    }
  }
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

inline bool oaz::games::ConnectFour::CheckDiagonalVictory2(
    const oaz::games::ConnectFour::Board& board, size_t row,
    size_t column) {
  Board mask = SECOND_DIAGONAL;
  size_t column_match = N_ROWS - 1 - row;
  if (column > column_match) {
    mask.PositiveColumnShift(column - column_match);
  } else {
    if (column < column_match) {
      mask.NegativeColumnShift(column_match - column);
    }
  }
  return board.LexicographicComponentLength(mask, row, column) >= 4;
}

bool oaz::games::ConnectFour::Player0Won() const { return m_status.test(0); }

bool oaz::games::ConnectFour::Player1Won() const { return m_status.test(1); }

float oaz::games::ConnectFour::GetScore() const {
  if (IsFinished()) {
    if (Player0Won()) {
      return 1;
    }
    if (Player1Won()) {
      return -1;
    }
  }
  return 0;
}

std::unique_ptr<oaz::games::Game> oaz::games::ConnectFour::Clone() const {
  return std::make_unique<ConnectFour>(*this);
}

bool oaz::games::ConnectFour::operator==(const ConnectFour& rhs) const {
  return m_player0_tokens == rhs.m_player0_tokens &&
         m_player1_tokens == rhs.m_player1_tokens && m_status == rhs.m_status;
}

void oaz::games::ConnectFour::WriteStateToTensorMemory(
    float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[N_ROWS][N_COLUMNS][N_PLAYERS]);
  const Board& player0_tokens = GetPlayerBoard(0);
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      tensor[i][j][0] = player0_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
  const Board& player1_tokens = GetPlayerBoard(1);
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      tensor[i][j][1] = player1_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
}

void oaz::games::ConnectFour::WriteCanonicalStateToTensorMemory(
    float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[N_ROWS][N_COLUMNS][N_PLAYERS]);
  const Board& current_player_tokens = GetPlayerBoard(GetCurrentPlayer());
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      tensor[i][j][0] = current_player_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
  const Board& other_player_tokens = GetPlayerBoard(1 - GetCurrentPlayer());
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      tensor[i][j][1] = other_player_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
}

void oaz::games::ConnectFour::InitialiseFromState(float* input_board) {
  Reset();
  size_t player_0 = 0;
  size_t player_1 = 1;

  Board& player0_tokens = GetPlayerBoard(player_0);
  Board& player1_tokens = GetPlayerBoard(player_1);

  boost::multi_array_ref<float, 3> data(input_board, boost::extents[N_ROWS][N_COLUMNS][N_PLAYERS]);

  for (int i = 0; i != N_ROWS; ++i) {
    for (int j = 0; j != N_COLUMNS; ++j) {
      if (data[i][j][0] == 1.0F) {
        player0_tokens.Set(i, j);
      } else {
	if (data[i][j][1] == 1.0F) {
          player1_tokens.Set(i, j);
        }
      }
    }
  }
  CheckVictory();
}

void oaz::games::ConnectFour::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 3> data(input_board, boost::extents[N_ROWS][N_COLUMNS][N_PLAYERS]);

  float current_player_count = 0.0;
  float other_player_count = 0.0;
  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      current_player_count += data[i][j][0];
      other_player_count += data[i][j][1];
    }
  }

  int current_player = current_player_count == other_player_count ? 0 : 1;
  int other_player = 1 - current_player;

  Board& current_player_tokens = GetPlayerBoard(current_player);
  Board& other_player_tokens = GetPlayerBoard(other_player);

  for (size_t i = 0; i != N_ROWS; ++i) {
    for (size_t j = 0; j != N_COLUMNS; ++j) {
      if (data[i][j][0] == 1.0F) {
        current_player_tokens.Set(i, j);
      } else {
        if (data[i][j][1] == 1.0F) {
          other_player_tokens.Set(i, j);
	}
      }
    }
  }
  CheckVictory();
}

uint64_t oaz::games::ConnectFour::GetState() const {
  uint64_t state = m_player0_tokens.GetBits();
  for (size_t i=0; i != N_COLUMNS; ++i) {
    state |= (m_player1_tokens.ColumnSum(i) << (N_SQUARES + 3*i));
  }
  return state;
}
