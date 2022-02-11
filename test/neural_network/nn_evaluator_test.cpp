#include <fstream>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define TEST_FRIENDS friend class EvaluationBatch_InitialiseElement_Test;

#include <chrono>
#include <string>
#include <thread>

#include "boost/multi_array.hpp"
#include "nlohmann/json.hpp"
#include "oaz/cache/simple_cache.hpp"
#include "oaz/games/connect_four.hpp"
#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/queue/queue.hpp"
#include "oaz/thread_pool/dummy_task.hpp"
#include "oaz/utils/utils.hpp"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using json = nlohmann::json;

namespace oaz::nn {

TEST(EvaluationBatch, Instantiation) { EvaluationBatch({6, 7, 2}, 64); }

TEST(EvaluationBatch, InitialiseElement) {
  oaz::thread_pool::DummyTask task;
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation(std::make_unique<oaz::nn::DefaultNNEvaluation>());
  oaz::games::ConnectFour game;
  EvaluationBatch batch({6, 7, 2}, 64);

  batch.InitialiseElement(0, &game, &evaluation, &task);

  auto dimensions = game.ClassMethods().GetBoardShape();
  for (int i = 0; i != dimensions[0]; ++i)
    for (int j = 0; j != dimensions[1]; ++j)
      for (int k = 0; k != dimensions[2]; ++k)
        ASSERT_FLOAT_EQ((batch.m_batch.template tensor<float, 4>()(0, i, j, k)),
                        0.);
}

TEST(EvaluationBatch, AcquireIndex) {
  EvaluationBatch batch({6, 7, 2}, 64);

  size_t index = batch.AcquireIndex();
  ASSERT_FLOAT_EQ(index, 0);

  index = batch.AcquireIndex();
  ASSERT_FLOAT_EQ(index, 1);
}

TEST(NNEvaluator, Instantiation) {
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  std::unique_ptr<tensorflow::Session> session(
      CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = CreateModel(session.get(), "input", "value", "policy");

  NNEvaluator evaluator(model, nullptr, pool, {6, 7, 2}, 64);
}

TEST(NNEvaluator, RequestEvaluation) {
  std::unique_ptr<tensorflow::Session> session(
      CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  NNEvaluator evaluator(model, nullptr, pool, {6, 7, 2}, 64);

  oaz::thread_pool::DummyTask task;
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation(std::make_unique<oaz::nn::DefaultNNEvaluation>());

  oaz::games::ConnectFour game;
  evaluator.RequestEvaluation(&game, &evaluation, &task);

  task.wait();
}

TEST(NNEvaluator, EvaluationWithCache) {
  std::unique_ptr<tensorflow::Session> session(
      CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  auto cache =
      std::make_shared<oaz::cache::SimpleCache>(oaz::games::ConnectFour(), 100);
  NNEvaluator evaluator(model, cache, pool, {6, 7, 2}, 64);

  oaz::thread_pool::DummyTask task;
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation(std::make_unique<oaz::nn::DefaultNNEvaluation>());

  oaz::games::ConnectFour game;
  evaluator.RequestEvaluation(&game, &evaluation, &task);

  task.wait();

  for (size_t i = 0; i != 50; ++i) {
    oaz::thread_pool::DummyTask task(1);
    evaluator.RequestEvaluation(&game, &evaluation, &task);
    task.wait();
  }

  ASSERT_EQ(cache->GetNumberOfHits(), 50);
}

TEST(NNEvaluator, EvaluationWithCacheLargeNumberOfRequests) {
  std::unique_ptr<tensorflow::Session> session(
      CreateSessionAndLoadGraph("frozen_model.pb"));
  auto model = CreateModel(session.get(), "input", "value", "policy");
  auto pool = std::make_shared<oaz::thread_pool::ThreadPool>(1);
  auto cache =
      std::make_shared<oaz::cache::SimpleCache>(oaz::games::ConnectFour(), 100);
  NNEvaluator evaluator(model, cache, pool, {6, 7, 2}, 64);

  oaz::thread_pool::DummyTask task;
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation(std::make_unique<oaz::nn::DefaultNNEvaluation>());
  oaz::games::ConnectFour game;
  evaluator.RequestEvaluation(&game, &evaluation, &task);

  task.wait();

  oaz::thread_pool::DummyTask task2(1000000);
  for (size_t i = 0; i != 1000000; ++i) {
    evaluator.RequestEvaluation(&game, &evaluation, &task2);
  }
  task2.wait();

  ASSERT_EQ(cache->GetNumberOfHits(), 1000000);
}

/* TEST (Inference, CheckResults) { */
/* 	oaz::thread_pool::ThreadPool pool(1); */
/* 	std::unique_ptr<tensorflow::Session>
 * session(createSessionAndLoadGraph("frozen_model.pb")); */
/* 	SharedModelPointer model(createModel(session.get(), "value",
 * "policy")); */
/* 	TestEvaluator evaluator(model, &pool, 64); */

/* 	std::ifstream ifs("data.json"); */
/* 	json data = json::parse(ifs); */

/* 	vector<typename Game::Value> values(data.size()); */
/* 	vector<typename Game::Policy> policies(data.size()); */
/* 	oaz::thread_pool::DummyTask task(data.size()); */

/* 	for(size_t i=0; i!= data.size(); ++i) { */
/* 		Game game; */
/* 		Game::Board& board = game.getBoard(); */
/* 		loadBoardFromJson<Game>(data[i]["input"], board); */

/* 		evaluator.requestEvaluation( */
/* 			&game, */
/* 			&values[i], */
/* 			&policies[i], */
/* 			&task */
/* 		); */

/* 		evaluator.forceEvaluation(); */
/* 		ASSERT_FLOAT_EQ(values[i], data[i]["value"]); */
/* 	} */

/* 	task.wait(); */
/* } */

/* TEST (Inference, DelayedEvaluation) { */
/* 	oaz::thread_pool::ThreadPool pool(1); */
/* 	std::unique_ptr<tensorflow::Session>
 * session(createSessionAndLoadGraph("frozen_model.pb")); */
/* 	SharedModelPointer model(createModel(session.get(), "value",
 * "policy")); */

/* 	size_t N_REQUESTS = 100; */
/* 	size_t BATCH_SIZE = 16; */
/* 	TestEvaluator evaluator(model, &pool, BATCH_SIZE); */

/* 	oaz::thread_pool::DummyTask task(N_REQUESTS); */

/* 	std::ifstream ifs("data.json"); */
/* 	json data = json::parse(ifs); */

/* 	size_t DATA_SIZE = data.size(); */

/* 	vector<Game> games(N_REQUESTS); */
/* 	vector<typename Game::Value> values(N_REQUESTS); */
/* 	vector<typename Game::Policy> policies(N_REQUESTS); */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) { */
/* 		Game::Board& board = games[i].getBoard(); */
/* 		loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board); */

/* 		evaluator.requestEvaluation( */
/* 			&games[i], */
/* 			&values[i], */
/* 			&policies[i], */
/* 			&task */
/* 		); */
/* 	} */

/* 	for(int i=0; i!= (N_REQUESTS / BATCH_SIZE) + 1; ++i) */
/* 		evaluator.forceEvaluation(); */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) */
/* 		ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]); */

/* 	task.wait(); */
/* } */

/* void makeEvaluationRequests( */
/* 	oaz::queue::SafeQueue<size_t>* queue, */
/* 	vector<Game>* games, */
/* 	vector<typename Game::Value>* values, */
/* 	vector<typename Game::Policy>* policies, */
/* 	TestEvaluator* evaluator, */
/* 	oaz::thread_pool::Task* task) { */

/* 	queue->lock(); */
/* 	while(!queue->empty()) { */
/* 		size_t index = queue->front(); */
/* 		queue->pop(); */
/* 		queue->unlock(); */

/* 		evaluator->requestEvaluation( */
/* 			&(*games)[index], */
/* 			&(*values)[index], */
/* 			&(*policies)[index], */
/* 			task */
/* 		); */

/* 		queue->lock(); */
/* 	} */
/* 	queue->unlock(); */
/* } */

/* TEST (Inference, MultiThreadedRequests) { */

/* 	size_t N_REQUESTS = 100; */
/* 	size_t BATCH_SIZE = 16; */
/* 	size_t N_THREADS = 2; */

/* 	oaz::thread_pool::ThreadPool pool(1); */

/* 	std::unique_ptr<tensorflow::Session>
 * session(createSessionAndLoadGraph("frozen_model.pb")); */
/* 	SharedModelPointer model(createModel(session.get(), "value",
 * "policy")); */

/* 	TestEvaluator evaluator(model, &pool, BATCH_SIZE); */

/* 	std::ifstream ifs("data.json"); */
/* 	json data = json::parse(ifs); */

/* 	size_t DATA_SIZE = data.size(); */

/* 	vector<Game> games(N_REQUESTS); */
/* 	vector<typename Game::Value> values(N_REQUESTS); */
/* 	vector<typename Game::Policy> policies(N_REQUESTS); */

/* 	oaz::queue::SafeQueue<size_t> queue; */
/* 	oaz::thread_pool::DummyTask task(N_REQUESTS); */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) { */
/* 		Game::Board& board = games[i].getBoard(); */
/* 		loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board); */
/* 		queue.push(i); */
/* 	} */

/* 	vector<std::thread> workers; */
/* 	for(size_t i=0; i!=2; ++i) { */
/* 		workers.push_back( */
/* 			std::thread( */
/* 				&makeEvaluationRequests, */
/* 				&queue, */
/* 				&games, */
/* 				&values, */
/* 				&policies, */
/* 				&evaluator, */
/* 				&task */
/* 			) */
/* 		); */
/* 	} */

/* 	for(size_t i=0; i!=N_THREADS; ++i) */
/* 		workers[i].join(); */

/* 	for(int i=0; i!= (N_REQUESTS / BATCH_SIZE) + 1; ++i) */
/* 		evaluator.forceEvaluation(); */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) */
/* 		ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]); */

/* 	task.wait(); */
/* } */

/* TEST (Inference, MultiThreadedRequestsAndEvaluations) { */

/* 	oaz::thread_pool::ThreadPool pool(1); */

/* 	size_t N_REQUESTS = 100; */
/* 	size_t BATCH_SIZE = 16; */
/* 	size_t N_THREADS = 2; */

/* 	oaz::thread_pool::DummyTask task(N_REQUESTS); */

/* 	std::unique_ptr<tensorflow::Session>
 * session(createSessionAndLoadGraph("frozen_model.pb")); */
/* 	SharedModelPointer model(createModel(session.get(), "value",
 * "policy")); */

/* 	TestEvaluator evaluator(model, &pool, BATCH_SIZE); */

/* 	std::ifstream ifs("data.json"); */
/* 	json data = json::parse(ifs); */

/* 	size_t DATA_SIZE = data.size(); */

/* 	vector<Game> games(N_REQUESTS); */
/* 	vector<typename Game::Value> values(N_REQUESTS); */
/* 	vector<typename Game::Policy> policies(N_REQUESTS); */

/* 	oaz::queue::SafeQueue<size_t> queue; */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) { */
/* 		Game::Board& board = games[i].getBoard(); */
/* 		loadBoardFromJson<Game>(data[i % DATA_SIZE]["input"], board); */
/* 		queue.push(i); */
/* 	} */

/* 	vector<std::thread> workers; */
/* 	for(size_t i=0; i!=2; ++i) { */
/* 		workers.push_back( */
/* 			std::thread( */
/* 				&makeEvaluationRequests, */
/* 				&queue, */
/* 				&games, */
/* 				&values, */
/* 				&policies, */
/* 				&evaluator, */
/* 				&task */
/* 			) */
/* 		); */

/* 	} */

/* 	for(size_t i=0; i!=N_THREADS; ++i) */
/* 		workers[i].join(); */

/* 	evaluator.forceEvaluation(); */

/* 	for(size_t i=0; i!= N_REQUESTS; ++i) */
/* 		ASSERT_FLOAT_EQ(values[i], data[i % DATA_SIZE]["value"]); */

/* 	task.wait(); */
/* } */
}  // namespace oaz::nn
