#include <string>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "boost/multi_array.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <memory>
#include <future>

#include "spdlog/spdlog.h"

using namespace oaz::nn;
using namespace tensorflow;
using namespace std;


template <class Game>
EvaluationBatch<Game>::EvaluationBatch(size_t size):
	m_current_index(0),
	m_n_reads(0),
	m_size(size),
	m_values(boost::extents[size]),
	m_policies(boost::extents[size]),
	m_tasks(boost::extents[size]) {

		auto dimensions = Game::Board::Dimensions();
		
		std::vector<long long int> tensor_dimensions = {(long long int) size};
		tensor_dimensions.insert(tensor_dimensions.end(), dimensions.begin(), dimensions.end());
		m_batch = tensorflow::Tensor(
			tensorflow::DT_FLOAT,
			tensorflow::TensorShape(tensor_dimensions)
		);
}

template <class Game>
void EvaluationBatch<Game>::readElementFromMemory(
	size_t index, 
	float* source,
	typename Game::Value* value,
	typename Game::Policy* policy,
	oaz::thread_pool::Task* task) {
	float* destination = m_batch.template tensor<float, Game::Board::NumDimensions() + 1>().data() + index * Game::Board::NumElements();
	std::memcpy(destination, source, Game::Board::SizeBytes());
	m_values[index] = value;
	m_policies[index] = policy;
	m_tasks[index] = task;
	++m_n_reads;
}




template <class Game>
bool EvaluationBatch<Game>::availableForEvaluation() {
	return m_current_index == m_n_reads;
}

template <class Game>
size_t EvaluationBatch<Game>::acquireIndex() {
	size_t index = m_current_index;
	++m_current_index;
	return	index;
}

template <class Game>
size_t EvaluationBatch<Game>::getSize() const {
	return m_size;
}

template <class Game>
tensorflow::Tensor& EvaluationBatch<Game>::getBatchTensor() {
	return m_batch;
}

template <class Game>
typename Game::Value* EvaluationBatch<Game>::getValue(size_t index) {
	return m_values[index];
}

template <class Game>
typename Game::Policy* EvaluationBatch<Game>::getPolicy(size_t index) {
	return m_policies[index];
}

template <class Game>
void EvaluationBatch<Game>::lock() {
	m_lock.lock();
}

template <class Game>
void EvaluationBatch<Game>::unlock() {
	m_lock.unlock();
}

template <class Game>
bool EvaluationBatch<Game>::full() {
	return m_current_index >= getSize();
}

template <class Game>
size_t EvaluationBatch<Game>::getNElements() const {
	return m_current_index;
}

template <class Game>
oaz::thread_pool::Task* EvaluationBatch<Game>::getTask(size_t index) {
	return m_tasks[index];
}

template <class Game>
NNEvaluator<Game>::NNEvaluator(
	SharedModelPointer model, 
	oaz::thread_pool::ThreadPool* thread_pool, 
	size_t batch_size): 
	m_batch_size(batch_size), 
	m_model(model),
	m_n_evaluation_requests(0),
	m_n_evaluations(0),
	m_thread_pool(thread_pool) {
		std::future<void> future_exit_signal = m_exit_signal.get_future();
		m_worker = std::thread(	
			&NNEvaluator::monitor,
			this,
			std::move(future_exit_signal)
		);
}

template <class Game>
NNEvaluator<Game>::~NNEvaluator() {
	m_exit_signal.set_value();
	m_worker.join();
}

template <class Game>
void NNEvaluator<Game>::monitor(std::future<void> future_exit_signal) {
	while(future_exit_signal.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout) {
		if(!m_evaluation_completed) {
			forceEvaluation();
		}
		m_evaluation_completed = false;
	}
};

template <class Game>
void NNEvaluator<Game>::addNewBatch() {
	UniqueBatchPointer batch(new Batch(m_batch_size));
	m_batches.push_back(std::move(batch));
}

template <class Game>
std::string NNEvaluator<Game>::getStatus() const {
	return "Evaluator status: " + std::to_string(m_n_evaluations * m_batch_size) + "/" + std::to_string(m_n_evaluation_requests * m_batch_size);
}


template <class Game>
void NNEvaluator<Game>::requestEvaluation(
	Game* game, 
	typename Game::Value* value,
	typename Game::Policy* policy,
	oaz::thread_pool::Task* task
	) {

	m_batches.lock();
	if(!m_batches.empty()) {
		Batch* current_batch = m_batches.back().get();
		current_batch->lock();
		if(!current_batch->full()) {
			size_t index = current_batch->acquireIndex();
			current_batch->unlock();
			m_batches.unlock();
			
			current_batch->readElementFromMemory(
				index,
				game->getBoard().origin(),
				value,
				policy,
				task
			);

		}
		else {
			UniqueBatchPointer current_batch_uniqueptr = std::move(m_batches.back());
			m_batches.pop_back();
			addNewBatch();
			current_batch->unlock();
			m_batches.unlock();
		
			while(!current_batch->availableForEvaluation());

			evaluateBatch(current_batch);

			requestEvaluation(
				game, 
				value,
				policy,
				task
			);
		}
	} else {
		addNewBatch();
		m_batches.unlock();
		requestEvaluation(
			game, 
			value,
			policy,
			task
		);
	}
}

template <class Game>
void NNEvaluator<Game>::evaluateBatch(Batch* batch) {

	spdlog::debug("Evaluating batch of size {}", batch->getNElements());
	std::vector<tensorflow::Tensor> outputs;

	m_n_evaluation_requests++;
	m_model->Run(
		{{"input:0", batch->getBatchTensor()}}, 
		{m_model->getValueNodeName(), m_model->getPolicyNodeName()},
		{},
		&outputs
	);
	m_n_evaluations++;

 	auto values_map = outputs[0].template tensor<float, 2>();
 	auto policies_map = outputs[1].template tensor<float, 2>();

	for(size_t i=0; i != batch->getNElements(); ++i) {
		std::memcpy(
			batch->getValue(i), 
			&values_map(i, 0),
			1 * sizeof(float)
		);

		std::memcpy(
			batch->getPolicy(i)->origin(), 
			&policies_map(i, 0),
			Game::Policy::SizeBytes()
		);
		
		m_thread_pool->enqueue(batch->getTask(i));
	}

}

template <class Game>
void NNEvaluator<Game>::forceEvaluation() {

	spdlog::debug("Forced evaluation");
	m_batches.lock();
	if (!m_batches.empty()) {
		Batch* earliest_batch = m_batches.front().get();
		earliest_batch->lock();
		if(earliest_batch->availableForEvaluation()) {
			UniqueBatchPointer earliest_batch_uniqueptr = std::move(m_batches.front());
			m_batches.pop_front();
			earliest_batch->unlock();
			m_batches.unlock();
			evaluateBatch(earliest_batch_uniqueptr.get());
		}
		else {
			earliest_batch->unlock();
			m_batches.unlock();
		}
	} else m_batches.unlock();
}
