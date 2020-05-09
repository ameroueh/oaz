#include "oaz/az/self_play.hpp"

#include <algorithm>
#include <iostream>
#include <stack>


template <class Game, class Evaluator, class SearchPool>
oaz::az::SelfPlay<Game, Evaluator, SearchPool>::SelfPlay(
	std::string file_path,
	SharedEvaluatorPointer evaluator,
	SharedSearchPoolPointer search_pool,
	size_t n_games,
	size_t n_simulations_per_move,
	size_t search_batch_size,
	size_t n_workers):
	m_file(file_path, H5F_ACC_TRUNC),
	m_search_pool(search_pool),
	m_evaluator(evaluator),
	m_n_games(n_games), 
	m_n_simulations_per_move(n_simulations_per_move),
	m_search_batch_size(search_batch_size), 
	m_n_workers(n_workers),
	m_counter(0)
	{
		initialise();
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::initialise() {}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::playGame(
	size_t n_simulations_per_move,
	size_t search_batch_size,
	size_t counter
) {

	Game game;

	auto board_dimensions = Game::getBoardDimensionsUnsigned();
	board_dimensions.insert(board_dimensions.begin(), Game::max_n_moves);
	boost::multi_array<float, 4> boards(
		boost::extents[Game::max_n_moves][Game::width][Game::height][Game::n_players]
	);
	boost::multi_array<float, 2> policies(boost::extents[Game::max_n_moves][Game::getPolicySize()]);
	boost::multi_array<float, 1> values(boost::extents[Game::max_n_moves]);


	size_t n_moves = 0;
	while(!game.Finished()) {

		boards[n_moves] = game.getBoard();	
		AZSearch<Game, Evaluator> search(
			game, 
			m_evaluator,
			search_batch_size, 
			n_simulations_per_move,
			0.25,
			1.0
		);

		m_search_pool->performSearch(&search);
		
		typename Game::Policy policy;
		search.getVisitCounts(policy);
		normaliseVisitCounts(policy);
	
		std::vector<typename Game::Move>* available_moves = game.availableMoves();
		typename Game::Move move = sampleMove(policy, available_moves);

		for(size_t i=0; i!=Game::getPolicySize(); ++i)
			policies[n_moves][i] = policy[i];
		
		game.playMove(move);
		++n_moves;
	}

	float score = game.score(); // 1 if first player won, -1 if second player won, 0 for a draw
	std::cout << "Score: " << score << " game length " << n_moves << std::endl;

	for(size_t i=0; i!=n_moves; ++i)
		values[i] = score;
	
	// Saving game to file
	std::string group_name = "/Game" + std::to_string(counter);
	m_file.createGroup(group_name);

	H5::DataSpace boards_space(board_dimensions.size(), board_dimensions.data());
	H5::DataSet dataset = m_file.createDataSet(group_name + "/Boards", H5::PredType::NATIVE_FLOAT, boards_space);
	dataset.write(boards.data(), H5::PredType::NATIVE_FLOAT); 	
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::normaliseVisitCounts(typename Game::Policy& visit_counts) {
	float total = 0.;
	for(size_t i=0; i!=visit_counts.size(); ++i)
		total += visit_counts[i];
	
	for(size_t i=0; i!=visit_counts.size(); ++i)
		visit_counts[i] /= total;
}

template <class Game, class Evaluator, class SearchPool>
typename Game::Move oaz::az::SelfPlay<Game, Evaluator, SearchPool>::sampleMove(typename Game::Policy& visit_counts, std::vector<typename Game::Move>* available_moves) {

	std::vector<float> available_moves_visit_counts;
	for(auto move : *available_moves)
		available_moves_visit_counts.push_back(visit_counts[move]);
	
	std::discrete_distribution<size_t> distribution(
		available_moves_visit_counts.begin(), 
		available_moves_visit_counts.end()
	);

	size_t move_index = distribution(m_generator);
	return (*available_moves)[move_index];
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::work() {
	size_t counter;
	while((counter = m_counter++) < m_n_games) {
		playGame(m_n_simulations_per_move, m_search_batch_size, counter);
	}
}

template <class Game, class Evaluator, class SearchPool>
void oaz::az::SelfPlay<Game, Evaluator, SearchPool>::playGames() {

	vector<std::thread> workers;
	
	for(size_t i = 0; i != m_n_workers; ++i) {
		workers.push_back(
			std::thread(
				&SelfPlay<Game, Evaluator, SearchPool>::work,
				this
			)
		);
	}

	for(size_t i=0; i!=m_n_workers; ++i)
		workers[i].join();
}

template <class Game, class Evaluator, class SearchPool>
std::string oaz::az::SelfPlay<Game, Evaluator, SearchPool>::getStatus() {
	size_t counter = std::min<size_t>(m_counter, m_n_games);
	return "Self-play progress: " + std::to_string(counter) + "/" + to_string(m_n_games);
}
