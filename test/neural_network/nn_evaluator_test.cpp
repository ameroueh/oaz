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
		
		auto dimensions = Game::Board::Dimensions();
		for(int i=0; i!=dimensions[0]; ++i)
			for(int j=0; j!=dimensions[1]; ++j) 
				for(int k=0; k!=dimensions[2]; ++k) 
					ASSERT_FLOAT_EQ(array[i][j][k], (batch.m_batch.template tensor<float, Game::Board::NumDimensions() + 1>()(0, i, j, k)));
	}

	TEST (EvaluationBatch, AcquireIndex) {
		EvaluationBatch<Game, DummyNotifier> batch(64);

		size_t index = batch.acquireIndex();
		ASSERT_FLOAT_EQ(index, 0);

		index = batch.acquireIndex();
		ASSERT_FLOAT_EQ(index, 1);
	}

	TEST (NNEvaluator, Instantiation) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		Evaluator evaluator(model, 64);
	}

	TEST (NNEvaluator, requestEvaluation) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
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
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
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

	TEST (Inference, CheckResults) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
		Evaluator evaluator(model, 64);

		std::ifstream ifs("data.json");
		json data = json::parse(ifs);

		vector<typename Game::Value> values(data.size());
		vector<typename Game::Policy> policies(data.size());
		
		for(size_t i=0; i!= data.size(); ++i) {
			Game game;
			Game::Board& board = game.getBoard();
			loadBoardFromJson<Game>(data[i]["input"], board);
			
			evaluator.requestEvaluation(
				&game,
				&values[i],
				&policies[i],
				DummyNotifier()	
			);
			
			evaluator.forceEvaluation();
			ASSERT_FLOAT_EQ(values[i], data[i]["value"]);
		}
	}
	
	TEST (Inference, DelayedEvaluation) {
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
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
			
			evaluator.requestEvaluation(
				&games[i],
				&values[i],
				&policies[i],
				DummyNotifier()	
			);
		}

		for(int i=0; i!= (N_REQUESTS / BATCH_SIZE) + 1; ++i)
			evaluator.forceEvaluation();
	
		for(size_t i=0; i!= N_REQUESTS; ++i)
			ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]);
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
		
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
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
			ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]);
	}
	
	TEST (Inference, MultiThreadedRequestsAndEvaluations) {

		size_t N_REQUESTS = 100;
		size_t BATCH_SIZE = 16;
		size_t N_THREADS = 2;
		
		std::unique_ptr<tensorflow::Session> session(createSessionAndLoadGraph("frozen_model.pb"));
		SharedModelPointer model(createModel(session.get(), "value", "policy"));
		
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
			ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]);
	}
}
