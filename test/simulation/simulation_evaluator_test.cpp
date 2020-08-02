#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/games/connect_four.hpp"
#include "oaz/simulation/simulation_evaluator.hpp"
#include "oaz/queue/queue.hpp"

#include <thread>
#include <vector>

using namespace std;

class DummyNotifier {
	public:
		void operator()() {}		
};

using Game = ConnectFour;
using Evaluator = SimulationEvaluator<Game, DummyNotifier>;

void playFromString(Game& game, std::string sMoves) {
	for(char& c : sMoves)
		game.playMove(c - '0');
}

TEST (Instantiation, Default) {
	Evaluator evaluator;
}

TEST (RequestEvaluation, Default) {
	Evaluator evaluator;
	
	Game game;
	Game game2(game);

	Game::Value value;
	Game::Policy policy;
	DummyNotifier notifier;

	evaluator.requestEvaluation(
		&game,
		&value,
		&policy,
		notifier
	);

	ASSERT_TRUE(game2 == game);

	game.playMove(0);
	game2.playMove(0);

	evaluator.requestEvaluation(
		&game,
		&value,
		&policy,
		notifier
	);
	ASSERT_TRUE(game2 == game);
}

void evaluateGames(
	std::vector<Game>* games,
	oaz::queue::SafeQueue<size_t>* indices_q, 
	DummyNotifier notifier, 
	Evaluator* evaluator, 
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


		playFromString(game, moves.substr(0, len));
		Game temp_game = game;

		evaluator->requestEvaluation(
			&game,
			&value,
			&policy,
			notifier
		);
	
		ASSERT_TRUE(game == temp_game);

		indices_q->lock();
	}
	indices_q->unlock();
		
}

TEST (RandomGames, Default) {

	std::vector<Game> games(1000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=1000; ++i) 
		indices.push(i);

	DummyNotifier notifier;
	Evaluator evaluator;
	
	evaluateGames(
		&games, 
		&indices, 
		notifier, 
		&evaluator,
		0
	);
}

TEST (MultithreadedRandomGames, Default) {
	
	std::vector<Game> games(1000);
	oaz::queue::SafeQueue<size_t> indices;
	for(size_t i=0; i!=1000; ++i) 
		indices.push(i);
	
	DummyNotifier notifier;
	Evaluator evaluator;

	vector<thread> threads; 
	for(size_t i=0; i!=2; ++i) {
		threads.push_back(
			thread(
				&evaluateGames, 
				&games,
				&indices,
				notifier,
				&evaluator,
				0
			)
		);
	}
	for(size_t i=0; i!=2; ++i)
		threads[i].join();
}

