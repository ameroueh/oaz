#include <string>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "oaz/neural_network/nn_evaluator.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <memory>
#include <future>

using namespace oaz::nn;
using namespace tensorflow;
using namespace std;

template <class Game, class Notifier>
NNEvaluator<Game, Notifier>::NNEvaluator(SharedModelPointer model, size_t batch_size): 
	m_batch_size(batch_size), 
	m_model(model),
	m_n_evaluation_requests(0),
	m_n_evaluations(0) {
		std::future<void> future_exit_signal = m_exit_signal.get_future();
		m_worker = std::thread(	
			&NNEvaluator::monitor,
			this,
			std::move(future_exit_signal)
		);
}

template <class Game, class Notifier>
NNEvaluator<Game, Notifier>::~NNEvaluator() {
	m_exit_signal.set_value();
	m_worker.join();
}

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::monitor(std::future<void> future_exit_signal) {
	while(future_exit_signal.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout) {
		if(!m_evaluation_completed) {
			forceEvaluation();
		}
		m_evaluation_completed = false;
	}
};

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::addNewBatch() {
	UniqueBatchPointer batch(new Batch(m_batch_size));
	m_batches.push_back(std::move(batch));
}

template <class Game, class Notifier>
std::string NNEvaluator<Game, Notifier>::getStatus() const {
	return "Evaluator status: " + std::to_string(m_n_evaluations * m_batch_size) + "/" + std::to_string(m_n_evaluation_requests * m_batch_size);
}


template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::requestEvaluation(
	Game* game, 
	typename Game::Value* value,
	typename Game::Policy* policy,
	Notifier notifier) {

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
				notifier
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
				notifier
			);
		}
	} else {
		addNewBatch();
		m_batches.unlock();
		requestEvaluation(
			game, 
			value,
			policy,
			notifier
		);
	}
}

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::evaluateBatch(Batch* batch) {
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

		batch->getNotifier(i)();
	}

}

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::forceEvaluation() {
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
