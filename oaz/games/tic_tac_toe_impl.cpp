#include <vector>

#include "oaz/games/tic_tac_toe.hpp"
#include <algorithm>
#include <iostream>
#include <string>

using namespace oaz::games;

TicTacToe::TicTacToe(): m_status(0) {}

void TicTacToe::PlayFromString(std::string moves) {
	for(char& c : moves) {
		PlayMove(c - '0');
	}
}

void TicTacToe::PlayMove(size_t move) {

	size_t row = move % 3;
	size_t column = move / 3;
	
	size_t player = GetCurrentPlayer();
	Board& board = GetPlayerBoard(player);
	board.Set(row, column);
	bool victory = CheckVictory(board, row, column);

	if(victory) {
		SetWinner(player);
		DeclareFinished();
	} else if ((m_player0_tokens | m_player1_tokens).Sum() == 9)
		DeclareFinished();
}

size_t TicTacToe::GetCurrentPlayer() const {
	return m_player0_tokens.Sum() == m_player1_tokens.Sum() ? 0 : 1;
}

const TicTacToe::Board& TicTacToe::GetPlayerBoard(size_t player) const {
	return (player == 0) ? m_player0_tokens : m_player1_tokens;
}

TicTacToe::Board& TicTacToe::GetPlayerBoard(size_t player) {
	return const_cast<Board&>(static_cast<const TicTacToe&>(*this).GetPlayerBoard(player));
}


bool TicTacToe::CheckVictory(Board& board, size_t row, size_t column) const {

	if((board.RowSum(row) == 3) || (board.ColumnSum(column) == 3)) {
		return true;
	}
	if((row == column) && ((board & FIRST_DIAGONAL).Sum() == 3)) {
		return true;
	}
	if((row == 2 - column) && ((board & SECOND_DIAGONAL).Sum() == 3)) {
		return true;
	}
	return false;
}

void TicTacToe::SetWinner(size_t player) {
	m_status.set(player);
}

void TicTacToe::DeclareFinished() {
	m_status.set(2);
}

void TicTacToe::GetAvailableMoves(std::vector<size_t>& moves) const {
	
	moves.clear();

	if(IsFinished())
		return;
	
	Board board = m_player0_tokens | m_player1_tokens;
	for(size_t move=0; move!= 9; ++move) {
		size_t row = move % 3;
		size_t column = move / 3;
		if(!board.Get(row, column))
			moves.push_back(move);
	}
}

bool TicTacToe::IsFinished() const {
	return m_status.test(2);
}

bool TicTacToe::Player0Won() const {
	return m_status.test(0);
}

bool TicTacToe::Player1Won() const {
	return m_status.test(1);
}

float TicTacToe::GetScore() const {
	if (IsFinished())
		if(Player0Won())
			return 1;
		else if (Player1Won())
			return -1;
		else return 0;
	else return 0;
}

bool TicTacToe::operator==(const TicTacToe& rhs) {
	return (m_player0_tokens == rhs.m_player0_tokens)
		&& (m_player1_tokens == rhs.m_player1_tokens)
		&& (m_status == rhs.m_status);
}

std::unique_ptr<Game> TicTacToe::Clone() const {
	return std::make_unique<TicTacToe>(*this);
}

void TicTacToe::WriteStateToTensorMemory(float* destination) const {
	boost::multi_array_ref<float, 3> tensor(
		destination,
		boost::extents[3][3][2]
	);
	const Board& player0_tokens = GetPlayerBoard(0);
	for(size_t i=0; i!=3; ++i)
		for(size_t j=0; j!=3; ++j)
			tensor[i][j][0] = player0_tokens.Get(i, j) ? 1. : 0.;
	const Board& player1_tokens = GetPlayerBoard(1);
	for(size_t i=0; i!=3; ++i)
		for(size_t j=0; j!=3; ++j)
			tensor[i][j][1] = player1_tokens.Get(i, j) ? 1. : 0.;
}

size_t TicTacToe::GetState() const {
	return m_player0_tokens.GetBits() | (m_player1_tokens.GetBits() << 9);	
}
