#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/thread_pool/dummy_task.hpp"
#include "oaz/queue/queue.hpp"

/* #include <thread> */
/* #include <vector> */

using namespace std;

TEST (Instantiation, Default) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(1);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
}

TEST (RequestEvaluation, Default) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(1);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
	ConnectFour game;
	boost::multi_array<float, 1> policy(boost::extents[7]);
	float value;
	
	oaz::thread_pool::DummyTask task(1);

	evaluator->RequestEvaluation(
		&game,
		&value,
		policy,
		&task
	);

	task.wait();
}

void EvaluateGames(
	std::vector<ConnectFour>* games,
	oaz::queue::SafeQueue<size_t>* indices_q, 
	oaz::thread_pool::Task* task, 
	std::shared_ptr<oaz::simulation::SimulationEvaluator> evaluator, 
	size_t thread_id) {
	std::string moves = "021302130213465640514455662233001144552636";
	size_t n_moves = moves.size();

	indices_q->Lock();
	while(!indices_q->empty()) {
		size_t index = indices_q->front();
		indices_q->pop();
		indices_q->Unlock();
		
		ConnectFour& game = (*games)[index];
	
		size_t len = index % (moves.size() + 1);

		game.PlayFromString(moves.substr(0, len));
		
		boost::multi_array<float, 1> policy(boost::extents[7]);
		float value;
		
		evaluator->RequestEvaluation(
			&game,
			&value,
			policy,
			task
		);
	
		indices_q->Lock();
	}
	indices_q->Unlock();
		
}

TEST (RandomGames, Default) {
	
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(1);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);

	std::vector<ConnectFour> games(10000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=10000; ++i) 
		indices.push(i);

	oaz::thread_pool::DummyTask task(10000);
	
	EvaluateGames(
		&games, 
		&indices, 
		&task, 
		evaluator,
		0
	);
	task.wait();
}

TEST (MultithreadedRandomGames, Default) {
	auto pool = make_shared<oaz::thread_pool::ThreadPool>(2);
	auto evaluator = make_shared<oaz::simulation::SimulationEvaluator>(pool);
	std::vector<ConnectFour> games(10000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=10000; ++i) 
		indices.push(i);
	oaz::thread_pool::DummyTask task(10000);
	
	vector<thread> threads; 
	for(size_t i=0; i!=2; ++i) {
		threads.push_back(
			thread(
				&EvaluateGames, 
				&games,
				&indices,
				&task,
				evaluator,
				0
			)
		);
	}
	for(size_t i=0; i!=2; ++i)
		threads[i].join();

	task.wait();
}
