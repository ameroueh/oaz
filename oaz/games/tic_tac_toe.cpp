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
  size_t row = move % SIDE_LENGTH;
  size_t column = move / SIDE_LENGTH;

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
    if ((m_player0_tokens | m_player1_tokens).Sum() == N_SQUARES) {DeclareFinished();}
  }
}

size_t oaz::games::TicTacToe::GetCurrentPlayer() const {
  return m_player0_tokens.Sum() == m_player1_tokens.Sum() ? 0 : 1;
}

template <class G>
auto& oaz::games::TicTacToe::GetPlayerBoardImpl(G& game, size_t player) {
  return (player == 0) ? game.m_player0_tokens : game.m_player1_tokens;
}

const oaz::games::TicTacToe::Board& oaz::games::TicTacToe::GetPlayerBoard(
    size_t player) const {
  return GetPlayerBoardImpl(*this, player);
}

oaz::games::TicTacToe::Board& oaz::games::TicTacToe::GetPlayerBoard(
    size_t player) {
  return GetPlayerBoardImpl(*this, player);
}

inline bool oaz::games::TicTacToe::CheckVictory(const Board& board, size_t row,
                                         size_t column) {
  if ((board.RowSum(row) == SIDE_LENGTH) || (board.ColumnSum(column) == SIDE_LENGTH)) {
    return true;
  }
  if ((row == column) && ((board & FIRST_DIAGONAL).Sum() == SIDE_LENGTH)) {
    return true;
  }
  if ((row == SIDE_LENGTH - 1 - column) && ((board & SECOND_DIAGONAL).Sum() == SIDE_LENGTH)) {
    return true;
  }
  return false;
}

inline bool oaz::games::TicTacToe::CheckVictory(const Board& board) {
  bool victory = false;
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      if (board.Get(i, j) == 1) {
        victory = CheckVictory(board, i, j);
        if (victory) {return victory;}
      }
    }
  }
  return victory;
}

inline void oaz::games::TicTacToe::CheckVictory() {
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
    std::vector<size_t>* moves) const {
  moves->clear();

  if (IsFinished()) {return;}

  Board board = m_player0_tokens | m_player1_tokens;
  for (size_t move = 0; move != N_SQUARES; ++move) {
    size_t row = move % SIDE_LENGTH;
    size_t column = move / SIDE_LENGTH;
    if (board.Get(row, column) == 0) {moves->push_back(move);}
  }
}

bool oaz::games::TicTacToe::IsFinished() const { return m_status.test(2); }

bool oaz::games::TicTacToe::Player0Won() const { return m_status.test(0); }

bool oaz::games::TicTacToe::Player1Won() const { return m_status.test(1); }

float oaz::games::TicTacToe::GetScore() const {
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

bool oaz::games::TicTacToe::operator==(const TicTacToe& rhs) {
  return (m_player0_tokens == rhs.m_player0_tokens) &&
         (m_player1_tokens == rhs.m_player1_tokens) &&
         (m_status == rhs.m_status);
}

std::unique_ptr<oaz::games::Game> oaz::games::TicTacToe::Clone() const {
  return std::make_unique<TicTacToe>(*this);
}

void oaz::games::TicTacToe::WriteStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, SIDE_LENGTH> tensor(destination, boost::extents[SIDE_LENGTH][SIDE_LENGTH][N_PLAYERS]);
  const Board& player0_tokens = GetPlayerBoard(0);
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      tensor[i][j][0] = player0_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
  const Board& player1_tokens = GetPlayerBoard(1);
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      tensor[i][j][1] = player1_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
}

void oaz::games::TicTacToe::WriteCanonicalStateToTensorMemory(
    float* destination) const {
  boost::multi_array_ref<float, 3> tensor(destination, boost::extents[SIDE_LENGTH][SIDE_LENGTH][N_PLAYERS]);
  const Board& current_player_tokens = GetPlayerBoard(GetCurrentPlayer());
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      tensor[i][j][0] = current_player_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
  const Board& other_player_tokens = GetPlayerBoard(1 - GetCurrentPlayer());
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      tensor[i][j][1] = other_player_tokens.Get(i, j) == 1 ? 1. : 0.;
    }
  }
}

void oaz::games::TicTacToe::InitialiseFromState(float* input_board) {
  Reset();
  size_t player_0 = 0;
  size_t player_1 = 1;

  Board& player0_tokens = GetPlayerBoard(player_0);
  Board& player1_tokens = GetPlayerBoard(player_1);

  boost::multi_array_ref<float, 3> data(input_board, boost::extents[SIDE_LENGTH][SIDE_LENGTH][N_PLAYERS]);

  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
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

void oaz::games::TicTacToe::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 3> data(input_board, boost::extents[SIDE_LENGTH][SIDE_LENGTH][N_PLAYERS]);

  float current_player_count = 0.0;
  float other_player_count = 0.0;
  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
      current_player_count += data[i][j][0];
      other_player_count += data[i][j][1];
    }
  }

  int current_player = current_player_count == other_player_count ? 0 : 1;
  int other_player = 1 - current_player;

  Board& current_player_tokens = GetPlayerBoard(current_player);
  Board& other_player_tokens = GetPlayerBoard(other_player);

  for (size_t i = 0; i != SIDE_LENGTH; ++i) {
    for (size_t j = 0; j != SIDE_LENGTH; ++j) {
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

size_t oaz::games::TicTacToe::GetState() const {
  return m_player0_tokens.GetBits() | (m_player1_tokens.GetBits() << N_SQUARES);
}
