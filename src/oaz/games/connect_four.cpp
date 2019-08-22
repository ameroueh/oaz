#include <vector>

#include "oaz/games/connect_four.hpp"

#include "boost/multi_array.hpp"


using namespace oaz::games;

typedef ConnectFour::tile_t tile_t;
typedef ConnectFour::move_t move_t;
typedef ConnectFour::gsize_t gsize_t;
typedef ConnectFour::board_t board_t;
typedef ConnectFour::tregistry_t tregistry_t;

ConnectFour::ConnectFour(): 
	m_current_player(0), 
	m_score(0),
	m_game_won(false),
	m_available_moves(0), 
	m_board(boost::extents[width][height][n_players]),
	m_tokens_in_column(boost::extents[width]) {
		initialise();
}


void ConnectFour::initialise() {
	for(move_t i = 0; i != width; ++i) 
		m_available_moves.push_back(i);
}


void ConnectFour::reset() {
	m_current_player = 0;
	m_score = 0;
	m_game_won = false;
	m_available_moves.resize(0);
	m_board = board_t(boost::extents[width][height][n_players]);
	m_tokens_in_column = tregistry_t(boost::extents[width]);

	initialise();
}


void ConnectFour::playMove(move_t move) {
	placeToken(move);
	maybeDeclareVictory(move);
	swapPlayers();
	refreshAvailableMoves();
}


void ConnectFour::undoMove(move_t move) {
	swapPlayers();
	m_game_won = false;
	removeToken(move);
	refreshAvailableMoves();
}


void ConnectFour::refreshAvailableMoves() {
	m_available_moves.resize(0);

	for(gsize_t i = 0; i!= width; ++i)
		if(m_tokens_in_column[i] < height)
			m_available_moves.push_back(i);
}


void ConnectFour::maybeDeclareVictory(move_t move) {
	gsize_t token_height = m_tokens_in_column[move] - 1;

	if (checkVerticalVictory(move, token_height, m_current_player)
	|| checkHorizontalVictory(move, token_height, m_current_player)
	|| checkFirstDiagonalVictory(move, token_height, m_current_player)
	|| checkSecondDiagonalVictory(move, token_height, m_current_player))
		m_game_won = true;
}
	
bool ConnectFour::checkVerticalVictory(gsize_t i, gsize_t j, tile_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; j - k >= 0; ++k) {
		if (m_board[i][j - k][player] != 1)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkHorizontalVictory(gsize_t i, gsize_t j, tile_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; i - k >= 0; ++k) {
		if (m_board[i - k][j][player] != 1)
			break;
		++counter;
	}
	for(gsize_t k = 0; i + 1 + k < width; ++k) {
		if (m_board[i + 1 + k][j][player] != 1)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkFirstDiagonalVictory(gsize_t i, gsize_t j, tile_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; (i - k >= 0) && (j - k >= 0); ++k) {
		if (m_board[i - k][j - k][player] != 1)
			break;
		++counter;
	}
	for(gsize_t k = 0; (i + 1 + k < width) && (j + 1 + k < height); ++k) {
		if (m_board[i + 1 + k][j + 1 + k][player] != 1)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkSecondDiagonalVictory(gsize_t i, gsize_t j, tile_t player) {
	gsize_t counter = 0;
	for(gsize_t k = 0; (i - k >= 0) && (j + k < height); ++k) {
		if (m_board[i - k][j + k][player] != 1)
			break;
		++counter;
	}
	for(gsize_t k = 0; (i + 1 + k < width) && (j - 1 - k >= 0); ++k) {
		if (m_board[i + 1 + k][j - 1 - k][player] != 1)
			break;
		++counter;
	}
	return counter >= 4;
}

void ConnectFour::swapPlayers() {
	m_current_player = (m_current_player == 1) ? 0 : 1;
}

void ConnectFour::placeToken(move_t move) {
	gsize_t tile_number = m_tokens_in_column[move];
	m_board[move][tile_number][m_current_player] = 1;
	++m_tokens_in_column[move];
}

void ConnectFour::removeToken(move_t move) {
	--m_tokens_in_column[move];
	gsize_t tile_number = m_tokens_in_column[move];
	m_board[move][tile_number][m_current_player] = 0;
}

bool ConnectFour::Finished() const {
	return (m_available_moves.size() == 0) || m_game_won;
}

std::vector<move_t>* ConnectFour::availableMoves() {return &m_available_moves; }

gsize_t ConnectFour::score() const {
	if (Finished() && m_game_won) {
		if(m_current_player == 1) 
			return 1;
		else
			return -1;
	}
	else 
		return 0;
}

tile_t ConnectFour::currentPlayer() const {return m_current_player; }


bool ConnectFour::operator==(const ConnectFour& rhs) {
	return (m_score == rhs.m_score)
	&& (m_game_won == rhs.m_game_won)
	&& (m_current_player == rhs.m_current_player)
	&& (m_available_moves == rhs.m_available_moves)
	&& (m_board == rhs.m_board)
	&& (m_tokens_in_column == rhs.m_tokens_in_column);
}
