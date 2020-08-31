#include <vector>

#include "oaz/games/bandits.hpp"
#include "boost/multi_array.hpp"
#include <algorithm>
#include <iostream>
#include <string>

using namespace oaz::games;

Bandits::Bandits(): 
	m_current_player(0), 
	m_score(0),
	m_game_won(false),
	m_available_moves(0) {
		initialise();
}

Bandits::Bandits(const Bandits& game):
	m_current_player(game.getCurrentPlayer()),
	m_score(game.m_score),
	m_game_won(game.m_game_won),
	m_available_moves(game.m_available_moves),
	m_board(game.m_board)
{
}

void Bandits::playFromString(std::string moves) {
	for(char& c : moves) {
		playMove(c - '0');
	}
}

void Bandits::setCurrentPlayer(size_t current_player) {
	m_current_player = current_player;
}

void Bandits::initialise() {
	for(Move i = 0; i != n_moves; ++i)
		m_available_moves.push_back(i);
	resetBoard();

}

void Bandits::reset() {
	setCurrentPlayer(0);
	m_score = 0;
	m_game_won = false;
	m_available_moves.resize(0);
	initialise();
}

void Bandits::resetBoard() {
	for(size_t i=0; i != Board::Dimensions()[0]; ++i)
		m_board[i] = EMPTY_TOKEN;
}

void Bandits::playMove(Move move) {
	
	placeToken(move);
	maybeDeclareVictory(move);
	swapPlayers();
	refreshAvailableMoves();
}


void Bandits::undoMove(Move move) {
	swapPlayers();
	m_game_won = false;
	removeToken(move);
	refreshAvailableMoves();
}


void Bandits::refreshAvailableMoves() {
	m_available_moves.resize(0);

	for(size_t i = 0; i!= Board::Dimensions()[0]; ++i)
		if(m_board[i] != BASE_TOKEN)
			m_available_moves.push_back(i);
}


void Bandits::maybeDeclareVictory(Move move) {
	m_game_won = true;
}
	
void Bandits::swapPlayers() {
	if (getCurrentPlayer() == 1)
		setCurrentPlayer(0);
	else
		setCurrentPlayer(1);
}

void Bandits::placeToken(Move move) {
	m_board[move] = BASE_TOKEN;
}

void Bandits::removeToken(Move move) {
	m_board[move] = EMPTY_TOKEN;
}

bool Bandits::Finished() const {
	return getCurrentPlayer() == 1;
}

std::vector<Bandits::Move>* Bandits::availableMoves() {
	return &m_available_moves;
}

float Bandits::score() const {
	for(size_t i=0; i != Board::Dimensions()[0]; ++i)
		if (m_board[i] == BASE_TOKEN)
			if ((i % 2) == 0)
				return 1.;
			else
				return -1.;
	return 0.;
}

size_t Bandits::currentPlayer() const {return m_current_player; }

bool Bandits::operator==(const Bandits& rhs) {
	return (m_score == rhs.m_score)
	&& (m_game_won == rhs.m_game_won)
	&& (getCurrentPlayer() == rhs.getCurrentPlayer())
	&& (m_available_moves == rhs.m_available_moves)
	&& (m_board == rhs.m_board);
}

size_t Bandits::getCurrentPlayer() const {
	return m_current_player;
}

void Bandits::set(const Bandits& game) {
	setCurrentPlayer(game.getCurrentPlayer());
	m_score = game.m_score;
	m_game_won = game.m_game_won;
	m_available_moves = game.m_available_moves;
	m_board = game.m_board;
}
