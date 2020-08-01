#include <vector>

#include "oaz/games/tic_tac_toe.hpp"
#include "boost/multi_array.hpp"
#include <algorithm>
#include <iostream>
#include <string>

using namespace oaz::games;

TicTacToe::TicTacToe(): 
	m_current_player(0), 
	m_score(0),
	m_game_won(false),
	m_available_moves(0) {
		initialise();
}

TicTacToe::TicTacToe(const TicTacToe& game):
	m_current_player(game.getCurrentPlayer()),
	m_score(game.m_score),
	m_game_won(game.m_game_won),
	m_available_moves(game.m_available_moves),
	m_board(game.m_board)
{
}

void TicTacToe::playFromString(std::string moves) {
	for(char& c : moves) {
		playMove(c - '0');
	}
}

void TicTacToe::setCurrentPlayer(size_t current_player) {
	m_current_player = current_player;
}

void TicTacToe::initialise() {
	for(Move i = 0; i != n_moves; ++i)
		m_available_moves.push_back(i);
	resetBoard();

}

void TicTacToe::reset() {
	setCurrentPlayer(0);
	m_score = 0;
	m_game_won = false;
	m_available_moves.resize(0);
	initialise();
}

void TicTacToe::resetBoard() {
	for(size_t i=0; i != Board::Dimensions()[0]; ++i)
		for(size_t j=0; j != Board::Dimensions()[1]; ++j)
			for(size_t k=0; k != Board::Dimensions()[2]; ++k)
				m_board[i][j][k] = EMPTY_TOKEN;
}

void TicTacToe::playMove(Move move) {
	
	placeToken(move);
	maybeDeclareVictory(move);
	swapPlayers();
	refreshAvailableMoves();
}


void TicTacToe::undoMove(Move move) {
	swapPlayers();
	m_game_won = false;
	removeToken(move);
	refreshAvailableMoves();
}


void TicTacToe::refreshAvailableMoves() {
	m_available_moves.resize(0);

	for(size_t j = 0; j!= Board::Dimensions()[1]; ++j)
		for(size_t i=0; i!=Board::Dimensions()[0]; ++i)
			if(m_board[i][j][0] != BASE_TOKEN && m_board[i][j][1] != BASE_TOKEN)
				m_available_moves.push_back(3*j + i);
}


void TicTacToe::maybeDeclareVictory(Move move) {
	size_t i = move % 3;
	size_t j = move / 3;

	if (checkVerticalVictory(i, m_current_player)
	|| checkHorizontalVictory(j, m_current_player)
	|| checkFirstDiagonalVictory(m_current_player)
	|| checkSecondDiagonalVictory(m_current_player))
		m_game_won = true;
}
	
bool TicTacToe::checkVerticalVictory(size_t i, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; k < Board::Dimensions()[1]; ++k) {
		if (m_board[i][k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter == 3;
}

bool TicTacToe::checkHorizontalVictory(size_t j, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; k < Board::Dimensions()[0]; ++k) {
		if (m_board[k][j][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter == 3;
}

bool TicTacToe::checkFirstDiagonalVictory(size_t player) {
	size_t counter = 0;
	for(size_t k = 0; k < 3; ++k) {
		if (m_board[k][k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter == 3;
}

bool TicTacToe::checkSecondDiagonalVictory(size_t player) {
	size_t counter = 0;
	for(size_t k = 0; k < 3; ++k) {
		if (m_board[k][2-k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter == 3;
}

void TicTacToe::swapPlayers() {
	if (getCurrentPlayer() == 1)
		setCurrentPlayer(0);
	else
		setCurrentPlayer(1);
}

void TicTacToe::placeToken(Move move) {
	size_t i = move % 3;
	size_t j = move / 3;
	m_board[i][j][m_current_player] = BASE_TOKEN;
}

void TicTacToe::removeToken(Move move) {
	size_t i = move % 3;
	size_t j = move / 3;
	m_board[i][j][m_current_player] = EMPTY_TOKEN;
}

bool TicTacToe::Finished() const {
	return (m_available_moves.size() == 0) || m_game_won;
}

std::vector<TicTacToe::Move>* TicTacToe::availableMoves() {
	return &m_available_moves;
}

float TicTacToe::score() const {
	if (Finished() && m_game_won) {
		if(getCurrentPlayer() == 1) 
			return 1;
		else
			return -1;
	}
	else 
		return 0;
}

size_t TicTacToe::currentPlayer() const {return m_current_player; }

bool TicTacToe::operator==(const TicTacToe& rhs) {
	return (m_score == rhs.m_score)
	&& (m_game_won == rhs.m_game_won)
	&& (getCurrentPlayer() == rhs.getCurrentPlayer())
	&& (m_available_moves == rhs.m_available_moves)
	&& (m_board == rhs.m_board);
}

size_t TicTacToe::getCurrentPlayer() const {
	return m_current_player;
}

void TicTacToe::set(const TicTacToe& game) {
	setCurrentPlayer(game.getCurrentPlayer());
	m_score = game.m_score;
	m_game_won = game.m_game_won;
	m_available_moves = game.m_available_moves;
	m_board = game.m_board;
}
