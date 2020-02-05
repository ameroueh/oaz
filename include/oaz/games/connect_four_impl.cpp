#include <vector>

/* #include "oaz/games/connect_four.hpp" */

#include "boost/multi_array.hpp"
#include <iostream>


using namespace oaz::games;


template <class Board>
ConnectFour<Board>::ConnectFour(boardptr_t board): 
	m_current_player(0), 
	m_score(0),
	m_game_won(false),
	m_available_moves(0), 
	m_board(board),
	m_tokens_in_column(boost::extents[width]) {
		initialise();
}


template <class Board>
void ConnectFour<Board>::initialise() {
	for(move_t i = 0; i != width; ++i) 
		m_available_moves.push_back(i);
	resetBoard();
}


template <class Board>
void ConnectFour<Board>::reset() {
	m_current_player = 0;
	m_score = 0;
	m_game_won = false;
	m_available_moves.resize(0);
	m_tokens_in_column = tregistry_t(boost::extents[width]);
	initialise();
}

template <class Board>
void ConnectFour<Board>::resetBoard() {
	for(gsize_t i=0; i != width; ++i)
		for(gsize_t j=0; j != height; ++j)
			for(gsize_t k=0; k != n_players; ++k)
				(*m_board)(i, j, k) = EMPTY_TOKEN;
}

template <class Board>
void ConnectFour<Board>::playMove(move_t move) {
	placeToken(move);
	maybeDeclareVictory(move);
	swapPlayers();
	refreshAvailableMoves();
}


template <class Board>
void ConnectFour<Board>::undoMove(move_t move) {
	swapPlayers();
	m_game_won = false;
	removeToken(move);
	refreshAvailableMoves();
}


template <class Board>
void ConnectFour<Board>::refreshAvailableMoves() {
	m_available_moves.resize(0);

	for(gsize_t i = 0; i!= width; ++i)
		if(m_tokens_in_column[i] < height)
			m_available_moves.push_back(i);
}


template <class Board>
void ConnectFour<Board>::maybeDeclareVictory(move_t move) {
	gsize_t token_height = m_tokens_in_column[move] - 1;

	if (checkVerticalVictory(move, token_height, m_current_player)
	|| checkHorizontalVictory(move, token_height, m_current_player)
	|| checkFirstDiagonalVictory(move, token_height, m_current_player)
	|| checkSecondDiagonalVictory(move, token_height, m_current_player))
		m_game_won = true;
}
	
template <class Board>
bool ConnectFour<Board>::checkVerticalVictory(gsize_t i, gsize_t j, gsize_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; j - k >= 0; ++k) {
		if ((*m_board)(i, j - k, player) != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

template <class Board>
bool ConnectFour<Board>::checkHorizontalVictory(gsize_t i, gsize_t j, gsize_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; i - k >= 0; ++k) {
		if ((*m_board)(i - k, j, player) != BASE_TOKEN)
			break;
		++counter;
	}
	for(gsize_t k = 0; i + 1 + k < width; ++k) {
		if ((*m_board)(i + 1 + k, j, player) != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

template <class Board>
bool ConnectFour<Board>::checkFirstDiagonalVictory(gsize_t i, gsize_t j, gsize_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; (i - k >= 0) && (j - k >= 0); ++k) {
		if ((*m_board)(i - k, j - k, player) != BASE_TOKEN)
			break;
		++counter;
	}
	for(gsize_t k = 0; (i + 1 + k < width) && (j + 1 + k < height); ++k) {
		if ((*m_board)(i + 1 + k, j + 1 + k, player) != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

template <class Board>
bool ConnectFour<Board>::checkSecondDiagonalVictory(gsize_t i, gsize_t j, gsize_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; (i - k >= 0) && (j + k < height); ++k) {
		if ((*m_board)(i - k, j + k, player) != BASE_TOKEN)
			break;
		++counter;
	}
	for(gsize_t k = 0; (i + 1 + k < width) && (j - 1 - k >= 0); ++k) {
		if ((*m_board)(i + 1 + k, j - 1 - k, player) != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

template <class Board>
void ConnectFour<Board>::swapPlayers() {
	m_current_player = (m_current_player == 1) ? 0 : 1;
}

template <class Board>
void ConnectFour<Board>::placeToken(move_t move) {
	gsize_t tile_number = m_tokens_in_column[move];
	(*m_board)(move, tile_number, m_current_player) = BASE_TOKEN;
	++m_tokens_in_column[move];
}

template <class Board>
void ConnectFour<Board>::removeToken(move_t move) {
	--m_tokens_in_column[move];
	gsize_t tile_number = m_tokens_in_column[move];
	(*m_board)(move, tile_number, m_current_player) = EMPTY_TOKEN;
}

template <class Board>
bool ConnectFour<Board>::Finished() const {
	return (m_available_moves.size() == 0) || m_game_won;
}

template <class Board>
std::vector<typename ConnectFour<Board>::move_t>* ConnectFour<Board>::availableMoves() {
	return &m_available_moves;
}

template <class Board>
float ConnectFour<Board>::score() const {
	if (Finished() && m_game_won) {
		if(m_current_player == 1) 
			return 1;
		else
			return -1;
	}
	else 
		return 0;
}

template <class Board>
typename ConnectFour<Board>::gsize_t ConnectFour<Board>::currentPlayer() const {return m_current_player; }

template <class Board>
bool ConnectFour<Board>::operator==(const ConnectFour<Board>& rhs) {
	bool boards_equal = true;
	for (gsize_t i=0; i!= width; ++i) 
		for (gsize_t j=0; j!=height; j++)
			for (gsize_t k=0; k!=n_players; k++)
				boards_equal &= ((*m_board)(i, j, k) == (*(rhs.m_board))(i, j, k));
	return (m_score == rhs.m_score)
	&& (m_game_won == rhs.m_game_won)
	&& (m_current_player == rhs.m_current_player)
	&& (m_available_moves == rhs.m_available_moves)
	&& boards_equal
	&& (m_tokens_in_column == rhs.m_tokens_in_column);
}
