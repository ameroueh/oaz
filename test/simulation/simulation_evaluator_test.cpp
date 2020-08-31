#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/queue/queue.hpp"
#include "oaz/thread_pool/dummy_task.hpp"

#include <thread>
#include <vector>

using namespace std;

using Game = ConnectFour;
using TestEvaluator = SimulationEvaluator<Game>;

TEST (Instantiation, Default) {
	oaz::thread_pool::ThreadPool pool(1);
	TestEvaluator evaluator(&pool);
}

TEST (RequestEvaluation, Default) {
	oaz::thread_pool::ThreadPool pool(1);
	TestEvaluator evaluator(&pool);
	
	Game game;
	Game game2(game);

	Game::Value value;
	Game::Policy policy;
	
	oaz::thread_pool::DummyTask task(2);

	evaluator.requestEvaluation(
		&game,
		&value,
		&policy,
		&task
	);

	ASSERT_TRUE(game2 == game);

	game.playMove(0);
	game2.playMove(0);

	evaluator.requestEvaluation(
		&game,
		&value,
		&policy,
		&task
	);
	ASSERT_TRUE(game2 == game);

	task.wait();
}

void evaluateGames(
	std::vector<Game>* games,
	oaz::queue::SafeQueue<size_t>* indices_q, 
	oaz::thread_pool::Task* task, 
	oaz::evaluator::Evaluator<Game>* evaluator, 
	size_t thread_id) {
	std::string moves = "021302130213465640514455662233001144552636";
	size_t n_moves = moves.size();

	indices_q->lock();
	while(!indices_q->empty()) {
		size_t index = indices_q->front();
		indices_q->pop();
		indices_q->unlock();
		
		Game& game = (*games)[index];
		typename Game::Value value;
		typename Game::Policy policy;
	
		size_t len = index % (moves.size() + 1);

		game.playFromString(moves.substr(0, len));
		Game temp_game = game;

		evaluator->requestEvaluation(
			&game,
			&value,
			&policy,
			task
		);
	
		ASSERT_TRUE(game == temp_game);

		indices_q->lock();
	}
	indices_q->unlock();
		
}

TEST (RandomGames, Default) {

	oaz::thread_pool::ThreadPool pool(1);
	std::vector<Game> games(1000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=1000; ++i) 
		indices.push(i);

	oaz::thread_pool::DummyTask task(1000);
	TestEvaluator evaluator(&pool);
	
	evaluateGames(
		&games, 
		&indices, 
		&task, 
		&evaluator,
		0
	);
	task.wait();
}

TEST (MultithreadedRandomGames, Default) {
	
	oaz::thread_pool::ThreadPool pool(1);
	std::vector<Game> games(1000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=1000; ++i) 
		indices.push(i);
	
	oaz::thread_pool::DummyTask task(1000);
	TestEvaluator evaluator(&pool);

	vector<thread> threads; 
	for(size_t i=0; i!=2; ++i) {
		threads.push_back(
			thread(
				&evaluateGames, 
				&games,
				&indices,
				&task,
				&evaluator,
				0
			)
		);
	}
	for(size_t i=0; i!=2; ++i)
		threads[i].join();

	task.wait();
}
