#include <fstream>
#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "oaz/neural_network/nn_testing.hpp"
#include "oaz/neural_network/nn_trainer.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
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
using Trainer = NNTrainer<Game>;

class DummyNotifier {
	public:
		void operator()(){}
};


using Evaluator = NNEvaluator<Game, DummyNotifier>;

namespace oaz::nn { 
	TEST (TrainingBatch, Instantiation) {
		TrainingBatch<Game>(16);
	}

	TEST (TrainingBatch, ReadElementFromMemory) {
		boost::multi_array<float, 3> board(boost::extents[7][6][2]);
		boost::multi_array<float, 1> policy_label(boost::extents[7]);
		float value_label = 0;
		TrainingBatch<Game> batch(16);
		batch.readElementFromMemory(
			0, 
			&board[0][0][0], 
			&value_label, 
			&policy_label[0]
		);
		
		vector<long long int> dimensions(Game::getBoardDimensions());
		for(int i=0; i!=dimensions[0]; ++i)
			for(int j=0; j!=dimensions[1]; ++j) 
				for(int k=0; k!=dimensions[2]; ++k) 
					ASSERT_EQ(board[i][j][k], (batch.getBoardTensor().template tensor<float, Game::NBoardDimensions + 1>()(0, i, j, k)));
		
		/* ASSERT_EQ(value, (batch.getValueTensor().template tensor<float, 1>()(0))); */
		ASSERT_EQ(value_label, (batch.getValueLabelTensor().template tensor<float, 1>()(0)));
		
		/* for(int i=0; i!=Game::getPolicySize(); ++i) */
		/* 	ASSERT_EQ(policy[i], (batch.getPolicyTensor().template tensor<float, 2>()(0, i))); */
		
		for(int i=0; i!=Game::getPolicySize(); ++i)
			ASSERT_EQ(policy_label[i], (batch.getPolicyLabelTensor().template tensor<float, 2>()(0, i)));
	}

	TEST (TrainingBatch, AcquireIndex) {
		TrainingBatch<Game> batch(16);

		size_t index = batch.acquireIndex();
		ASSERT_EQ(index, 0);

		index = batch.acquireIndex();
		ASSERT_EQ(index, 1);
	}

	TEST (NNTrainer, Instantiation) {

		Trainer trainer(16, 1);
	}

	TEST (NNTrainer, LoadModel) {

		Trainer trainer(16, 1);
		trainer.load_model("training_model");
	}
	
	TEST (NNTrainer, addTrainingExample) {

		Trainer trainer(16, 1);
		trainer.load_model("training_model");
		
		boost::multi_array<float, 3> board(boost::extents[7][6][2]);
		typename Game::Policy policy_label;
		float value_label = 0;
		
		trainer.addTrainingExample(
			&board, &value_label, &policy_label
		);
	}
	
	TEST (NNTrainer, implicitTraining) {

		Trainer trainer(1, 1);
		trainer.load_model("training_model");
		
		boost::multi_array<float, 3> board(boost::extents[7][6][2]);
		typename Game::Policy policy_label;
		float value_label = 0;
		
		trainer.addTrainingExample(
			&board, &value_label, &policy_label
		);
	}
	
	TEST (NNTrainer, largeBatchTraining) {

		Trainer trainer(7, 1);
		trainer.load_model("training_model");
		
		boost::multi_array<float, 3> board(boost::extents[7][6][2]);
		typename Game::Policy policy_label;
		float value_label = 0;
		
		for(int i=0; i!=7; ++i) 
			trainer.addTrainingExample(
				&board, &value_label, &policy_label
			);
	}
	
	TEST (NNTrainer, checkpoint) {

		Trainer trainer(7, 1);
		trainer.load_model("training_model");
		trainer.checkpoint("trained_model");
		
	}
	
	TEST (NNTrainer, CheckResults) {

		std::ifstream ifs("training_data.json");
		json data = json::parse(ifs);

		size_t batch_size = data["data"]["batch_size"];
		
		Trainer trainer(batch_size, 1);
		trainer.load_model("training_model");
		
		vector< typename Game::Board > boards;
		for (size_t i=0; i!=batch_size; ++i) {
			boost::multi_array<float, 3> board(boost::extents[7][6][2]);
			loadBoardFromJson<Game>(data["data"]["input"][i], board);
			boards.push_back(board);
		}
		
		vector< typename Game::Policy > policy_labels;
		for (size_t i=0; i!=batch_size; ++i) {
			typename Game::Policy policy_label;
			loadArrayFromJson(data["data"]["policy_labels"][i], policy_label);
			policy_labels.push_back(policy_label);
		}
		
		vector< float > value_labels;
		for (size_t i=0; i!=batch_size; ++i) {
			value_labels.push_back(data["data"]["value_labels"][i]);
		}
		
		size_t n_epochs = data["n_epochs"];

		for(size_t i=0; i!=n_epochs; ++i) {
			for (size_t j=0; j!=batch_size; ++j) {
				trainer.addTrainingExample(
					&boards[j],
					&value_labels[j],
					&policy_labels[j]
				);
			}
		}

		trainer.checkpoint("trained_model");

		Evaluator evaluator(64);
		evaluator.load_model("trained_model");

		std::ifstream ifs2("data_after_training.json");
		json data_after_training = json::parse(ifs2);

		vector<typename Game::Value> values(data_after_training.size());
		vector<typename Game::Policy> policies(data_after_training.size());
		
		for(size_t i=0; i!= data_after_training.size(); ++i) {
			Game game;
			Game::Board& board = game.getBoard();
			loadBoardFromJson<Game>(data_after_training[i]["input"], board);
			
			evaluator.requestEvaluation(
				&game,
				&values[i],
				&policies[i],
				DummyNotifier()	
			);
			
			evaluator.forceEvaluation();
			ASSERT_EQ(values[i], data_after_training[i]["value"]);
		}
	}

	void addTrainingExamples(Trainer* trainer, size_t n_iterations) {
		boost::multi_array<float, 3> board(boost::extents[7][6][2]);
		typename Game::Policy policy_label;
		float value_label = 0;
		
		for(size_t i=0; i!=n_iterations; ++i)
			trainer->addTrainingExample(
				&board, &value_label, &policy_label
			);
	}
	
	TEST (NNTrainer, MultiThreaded) {
		
		size_t N_ITERATIONS=100;
		size_t N_THREADS=2;

		Trainer trainer(16, 1);
		trainer.load_model("training_model");

		vector<std::thread> workers;
		for(size_t i=0; i!=N_THREADS; ++i)
			workers.push_back(
				std::thread(
					&addTrainingExamples,
					&trainer,
					N_ITERATIONS
				)
			);
		
		for(size_t i=0; i!=N_THREADS; ++i)
			workers[i].join();
	}
}
