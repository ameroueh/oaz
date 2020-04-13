#include <vector>

#include "oaz/games/connect_four.hpp"
#include "boost/multi_array.hpp"


using namespace oaz::games;


ConnectFour::ConnectFour(): 
	m_current_player(0), 
	m_score(0),
	m_game_won(false),
	m_available_moves(0), 
	m_board(boost::extents[width][height][n_players]) {
		initialise();
}

ConnectFour::ConnectFour(const ConnectFour& game):
	m_current_player(game.getCurrentPlayer()),
	m_score(game.m_score),
	m_game_won(game.m_game_won),
	m_available_moves(game.m_available_moves),
	m_tokens_in_column(game.m_tokens_in_column),
	m_board(game.m_board)
{
	/* for(int i=0; i!=width; ++i) */
	/* 	for(int j=0; j!=height; ++j) */
	/* 		for(int k=0; k!=n_players; ++k) */
	/* 			m_board(i, j, k) = game.m_board(i, j, k); */
}

void ConnectFour::setCurrentPlayer(size_t current_player) {
	m_current_player = current_player;
}

void ConnectFour::initialise() {
	/* std::cout << "Initialising game" << std::endl; */
	for(Move i = 0; i != width; ++i) {
		m_available_moves.push_back(i);
		m_tokens_in_column[i] = 0;
	}
	resetBoard();
	/* std::cout << "Game initialised" << std::endl; */

}

void ConnectFour::reset() {
	setCurrentPlayer(0);
	m_score = 0;
	m_game_won = false;
	m_available_moves.resize(0);
	m_tokens_in_column = Registry();
	initialise();
}

void ConnectFour::resetBoard() {
	for(size_t i=0; i != width; ++i)
		for(size_t j=0; j != height; ++j)
			for(size_t k=0; k != n_players; ++k)
				m_board[i][j][k] = EMPTY_TOKEN;
}

void ConnectFour::playMove(Move move) {
	placeToken(move);
	maybeDeclareVictory(move);
	swapPlayers();
	refreshAvailableMoves();
}


void ConnectFour::undoMove(Move move) {
	swapPlayers();
	m_game_won = false;
	removeToken(move);
	refreshAvailableMoves();
}


void ConnectFour::refreshAvailableMoves() {
	m_available_moves.resize(0);

	for(size_t i = 0; i!= width; ++i)
		if(m_tokens_in_column[i] < height)
			m_available_moves.push_back(i);
}


void ConnectFour::maybeDeclareVictory(Move move) {
	size_t token_height = m_tokens_in_column[move] - 1;

	if (checkVerticalVictory(move, token_height, m_current_player)
	|| checkHorizontalVictory(move, token_height, m_current_player)
	|| checkFirstDiagonalVictory(move, token_height, m_current_player)
	|| checkSecondDiagonalVictory(move, token_height, m_current_player))
		m_game_won = true;
}
	
bool ConnectFour::checkVerticalVictory(size_t i, size_t j, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; j >= k; ++k) {
		if (m_board[i][j - k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkHorizontalVictory(size_t i, size_t j, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; i >= k; ++k) {
		if (m_board[i - k][j][player] != BASE_TOKEN)
			break;
		++counter;
	}
	for(size_t k = 0; i + 1 + k < width; ++k) {
		if (m_board[i + 1 + k][j][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkFirstDiagonalVictory(size_t i, size_t j, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; (i >= k) && (j >= k); ++k) {
		if (m_board[i - k][j - k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	for(size_t k = 0; (i + 1 + k < width) && (j + 1 + k < height); ++k) {
		if (m_board[i + 1 + k][j + 1 + k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

bool ConnectFour::checkSecondDiagonalVictory(size_t i, size_t j, size_t player) {
	size_t counter = 0;
	for(size_t k = 0; (i >= k) && (j + k < height); ++k) {
		if (m_board[i - k][j + k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	for(size_t k = 0; (i + 1 + k < width) && (j - 1 >= k); ++k) {
		if (m_board[i + 1 + k][j - 1 - k][player] != BASE_TOKEN)
			break;
		++counter;
	}
	return counter >= 4;
}

void ConnectFour::swapPlayers() {
	if (getCurrentPlayer() == 1)
		setCurrentPlayer(0);
	else
		setCurrentPlayer(1);
}

void ConnectFour::placeToken(Move move) {
	size_t tile_number = m_tokens_in_column[move];
	m_board[move][tile_number][m_current_player] = BASE_TOKEN;
	++m_tokens_in_column[move];
}

void ConnectFour::removeToken(Move move) {
	--m_tokens_in_column[move];
	size_t tile_number = m_tokens_in_column[move];
	m_board[move][tile_number][m_current_player] = EMPTY_TOKEN;
}

bool ConnectFour::Finished() const {
	return (m_available_moves.size() == 0) || m_game_won;
}

std::vector<ConnectFour::Move>* ConnectFour::availableMoves() {
	return &m_available_moves;
}

float ConnectFour::score() const {
	if (Finished() && m_game_won) {
		if(getCurrentPlayer() == 1) 
			return 1;
		else
			return -1;
	}
	else 
		return 0;
}

size_t ConnectFour::currentPlayer() const {return m_current_player; }

bool ConnectFour::operator==(const ConnectFour& rhs) {
	bool boards_equal = true;
	for (size_t i=0; i!= width; ++i) 
		for (size_t j=0; j!=height; j++)
			for (size_t k=0; k!=n_players; k++)
				boards_equal &= (m_board[i][j][k] == rhs.m_board[i][j][k]);
	return (m_score == rhs.m_score)
	&& (m_game_won == rhs.m_game_won)
	&& (getCurrentPlayer() == rhs.getCurrentPlayer())
	&& (m_available_moves == rhs.m_available_moves)
	&& boards_equal
	&& (m_tokens_in_column == rhs.m_tokens_in_column);
}

size_t ConnectFour::getCurrentPlayer() const {
	return m_current_player;
}

void ConnectFour::set(const ConnectFour& game) {
	setCurrentPlayer(game.getCurrentPlayer());
	m_score = game.m_score;
	m_game_won = game.m_game_won;
	m_available_moves = game.m_available_moves;
	m_tokens_in_column = game.m_tokens_in_column;
	for (size_t i=0; i!= width; ++i) 
		for (size_t j=0; j!=height; j++)
			for (size_t k=0; k!=n_players; k++)
				m_board[i][j][k] = game.m_board[i][j][k];
}
