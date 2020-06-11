#include <fstream>
#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_FRIENDS \
	friend class EvaluationBatch_ReadElementFromMemory_Test ;


#include "oaz/neural_network/nn_testing.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/neural_network/model.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/queue/queue.hpp"

#include "nlohmann/json.hpp"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include "oaz/utils/utils.hpp"

#include <chrono>
#include <string>
#include <thread>

#include "boost/multi_array.hpp"

using namespace tensorflow;
using namespace oaz::nn;
using json = nlohmann::json;
using Game = ConnectFour;
using Model = oaz::nn::Model;

using SharedModelPointer = std::shared_ptr<Model>;

class DummyNotifier {
	public:
		void operator()(){}
};


using Evaluator = NNEvaluator<Game, DummyNotifier>;

namespace oaz::nn {

	TEST (EvaluationBatch, Instantiation) {
		EvaluationBatch<Game, DummyNotifier>(64);
	}

	TEST (EvaluationBatch, ReadElementFromMemory) {
		DummyNotifier notifier;
		boost::multi_array<float, 3> array(boost::extents[7][6][2]);
		EvaluationBatch<Game, DummyNotifier> batch(64);
		batch.readElementFromMemory(
			0, 
			&array[0][0][0], 
			nullptr, 
			nullptr, 
			notifier
		);
		
		vector<long long int> dimensions(Game::getBoardDimensions());
		for(int i=0; i!=dimensions[0]; ++i)
			for(int j=0; j!=dimensions[1]; ++j) 
				for(int k=0; k!=dimensions[2]; ++k) 
					ASSERT_EQ(array[i][j][k], (batch.m_batch.template tensor<float, Game::NBoardDimensions + 1>()(0, i, j, k)));
	}

	TEST (EvaluationBatch, AcquireIndex) {
		EvaluationBatch<Game, DummyNotifier> batch(64);

		size_t index = batch.acquireIndex();
		ASSERT_EQ(index, 0);

		index = batch.acquireIndex();
		ASSERT_EQ(index, 1);
	}

	TEST (NNEvaluator, Instantiation) {
		
		SharedModelPointer model(new Model());
		model->Load("model");

		Evaluator evaluator(model, 64);
	}

	TEST (NNEvaluator, requestEvaluation) {
		
		SharedModelPointer model(new Model());
		model->Load("model");
		
		DummyNotifier notifier;
		typename Game::Value value;
		typename Game::Policy policy;

		Evaluator evaluator(model, 64);
		Game game;
		evaluator.requestEvaluation(
			&game,
			&value,
			&policy,
			notifier
		);
	}

	TEST (NNEvaluator, forceEvaluation) {
		
		SharedModelPointer model(new Model());
		model->Load("model");
		
		DummyNotifier notifier;
		typename Game::Value value;
		typename Game::Policy policy;

		Evaluator evaluator(model, 64);
		Game game;
		evaluator.requestEvaluation(
			&game,
			&value,
			&policy,
			notifier
		);

		evaluator.forceEvaluation();
	}
	
	/* TEST (NNEvaluator, playMoves) { */
		
	/* 	SharedModelPointer model(new Model()); */
	/* 	model->Load("model"); */
		
	/* 	DummyNotifier notifier; */
	/* 	typename Game::Value value; */
	/* 	typename Game::Policy policy; */

	/* 	Evaluator evaluator(model, 64); */
	/* 	Game game; */
		
	/* 	game.playMove(0); */
	/* 	game.playMove(2); */
	/* 	game.playMove(3); */

	/* 	evaluator.requestEvaluation( */
	/* 		&game, */
	/* 		&value, */
	/* 		&policy, */
	/* 		notifier */
	/* 	); */

	/* 	evaluator.forceEvaluation(); */

	/* 	std::cout << "Value is " << value << std::endl; */
	/* } */

	TEST (Inference, CheckResults) {
		
		SharedModelPointer model(new Model());
		model->Load("model");
		
		Evaluator evaluator(model, 64);

		std::ifstream ifs("data.json");
		json data = json::parse(ifs);

		vector<typename Game::Value> values(data.size());
		vector<typename Game::Policy> policies(data.size());
		
		for(size_t i=0; i!= data.size(); ++i) {
			Game game;
			Game::Board& board = game.getBoard();
			loadBoardFromJson<Game>(data[i]["input"], board);
			
			/* std::cout << "Value is at " << &values[i] << std::endl; */
			evaluator.requestEvaluation(
				&game,
				&values[i],
				&policies[i],
				DummyNotifier()	
			);
			
			evaluator.forceEvaluation();
			ASSERT_EQ(values[i], data[i]["value"]);
			/* ASSERT_TRUE(evaluator.getPolicy(0) == data[i]["policy"]); */
		}
	}
	
	TEST (Inference, DelayedEvaluation) {
		
		SharedModelPointer model(new Model());
		model->Load("model");

		size_t N_REQUESTS = 100;
		size_t BATCH_SIZE = 16;
		Evaluator evaluator(model, BATCH_SIZE);

		std::ifstream ifs("data.json");
		json data = json::parse(ifs);

		size_t DATA_SIZE = data.size();

		vector<Game> games(N_REQUESTS);
		vector<typename Game::Value> values(N_REQUESTS);
		vector<typename Game::Policy> policies(N_REQUESTS);
		
		for(size_t i=0; i!= N_REQUESTS; ++i) {
			Game::Board& board = games[i].getBoard();
			loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board);
			
			/* std::cout << "Value is at " << &values[i] << std::endl; */
			evaluator.requestEvaluation(
				&games[i],
				&values[i],
				&policies[i],
				DummyNotifier()	
			);
			
			/* ASSERT_TRUE(evaluator.getPolicy(0) == data[i]["policy"]); */
		}

		for(int i=0; i!= (N_REQUESTS / BATCH_SIZE) + 1; ++i)
			evaluator.forceEvaluation();
	
		for(size_t i=0; i!= N_REQUESTS; ++i)
			ASSERT_EQ(values[i], data[i % DATA_SIZE]["value"]);
	}

	void makeEvaluationRequests(
		oaz::queue::SafeQueue<size_t>* queue, 
		vector<Game>* games,
		vector<typename Game::Value>* values,
		vector<typename Game::Policy>* policies,
		Evaluator* evaluator) {
		
		queue->lock();
		while(!queue->empty()) {
			size_t index = queue->front();
			queue->pop();
			queue->unlock();
			
			evaluator->requestEvaluation(
				&(*games)[index],
				&(*values)[index],
				&(*policies)[index],
				DummyNotifier()
			);

			queue->lock();
		}
		queue->unlock();
	}


	TEST (Inference, MultiThreadedRequests) {

		size_t N_REQUESTS = 100;
		size_t BATCH_SIZE = 16;
		size_t N_THREADS = 2;
		
		SharedModelPointer model(new Model());
		model->Load("model");

		Evaluator evaluator(model, BATCH_SIZE);

		std::ifstream ifs("data.json");
		json data = json::parse(ifs);

		size_t DATA_SIZE = data.size();

		vector<Game> games(N_REQUESTS);
		vector<typename Game::Value> values(N_REQUESTS);
		vector<typename Game::Policy> policies(N_REQUESTS);

		oaz::queue::SafeQueue<size_t> queue;
		
		for(size_t i=0; i!= N_REQUESTS; ++i) {
			Game::Board& board = games[i].getBoard();
			loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board);
			queue.push(i);
		}
		
		vector<std::thread> workers;
		for(size_t i=0; i!=2; ++i) {
			workers.push_back(
				std::thread(
					&makeEvaluationRequests,
					&queue,
					&games,
					&values,
					&policies,
					&evaluator
				)
			);
		}

		for(size_t i=0; i!=N_THREADS; ++i)
			workers[i].join();
		
		for(int i=0; i!= (N_REQUESTS / BATCH_SIZE) + 1; ++i)
			evaluator.forceEvaluation();
		
		for(size_t i=0; i!= N_REQUESTS; ++i)
			ASSERT_EQ(values[i], data[i % DATA_SIZE]["value"]);
	}
	
	TEST (Inference, MultiThreadedRequestsAndEvaluations) {

		size_t N_REQUESTS = 100;
		size_t BATCH_SIZE = 16;
		size_t N_THREADS = 2;
		
		SharedModelPointer model(new Model());
		model->Load("model");

		Evaluator evaluator(model, BATCH_SIZE);

		std::ifstream ifs("data.json");
		json data = json::parse(ifs);

		size_t DATA_SIZE = data.size();

		vector<Game> games(N_REQUESTS);
		vector<typename Game::Value> values(N_REQUESTS);
		vector<typename Game::Policy> policies(N_REQUESTS);

		oaz::queue::SafeQueue<size_t> queue;
		
		for(size_t i=0; i!= N_REQUESTS; ++i) {
			Game::Board& board = games[i].getBoard();
			loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board);
			queue.push(i);
		}
		
		vector<std::thread> workers;
		for(size_t i=0; i!=2; ++i) {
			workers.push_back(
				std::thread(
					&makeEvaluationRequests,
					&queue,
					&games,
					&values,
					&policies,
					&evaluator
				)
			);
			
		}

		for(size_t i=0; i!=N_THREADS; ++i)
			workers[i].join();
		
		evaluator.forceEvaluation();
		
		for(size_t i=0; i!= N_REQUESTS; ++i)
			ASSERT_EQ(values[i], data[i % DATA_SIZE]["value"]);
	}
}
