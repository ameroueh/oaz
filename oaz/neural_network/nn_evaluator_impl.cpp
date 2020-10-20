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

/* #include "spdlog/spdlog.h" */

using namespace oaz::nn;
using namespace tensorflow;
using namespace std;



EvaluationBatch::EvaluationBatch(
	const std::vector<int>& element_dimensions,
	size_t size):
	m_current_index(0),
	m_n_reads(0),
	m_size(size),
	m_element_size(
		std::accumulate(
			element_dimensions.cbegin(),
			element_dimensions.cend(),
			1,
			std::multiplies<int>()
		)
	),
	m_games(boost::extents[size]),
	m_values(boost::extents[size]),
	m_policies(boost::extents[size]),
	m_tasks(boost::extents[size]) {

		std::vector<long long int> tensor_dimensions = {(long long int) size};
		tensor_dimensions.insert(
			tensor_dimensions.end(), 
			element_dimensions.begin(), 
			element_dimensions.end()
		);
		m_batch = tensorflow::Tensor(
			tensorflow::DT_FLOAT,
			tensorflow::TensorShape(tensor_dimensions)
		);
}


void EvaluationBatch::InitialiseElement(
	size_t index, 
	oaz::games::Game* game,
	float* value,
	boost::multi_array_ref<float, 1> policy,
	oaz::thread_pool::Task* task) {
	float* destination = m_batch.flat<float>().data() + index * GetElementSize();
	game->WriteStateToTensorMemory(destination);
	m_games[index] = game;
	m_values[index] = value;
	m_policies[index] = std::move(
		std::make_unique<boost::multi_array_ref<float, 1>>(policy)
	);
	m_tasks[index] = task;
	++m_n_reads;
}

bool EvaluationBatch::IsAvailableForEvaluation() const {
	return m_current_index == m_n_reads;
}


size_t EvaluationBatch::AcquireIndex() {
	size_t index = m_current_index;
	++m_current_index;
	return	index;
}

boost::multi_array_ref<oaz::games::Game*, 1> EvaluationBatch::GetGames() {
	return m_games;
}

boost::multi_array_ref<float*, 1> EvaluationBatch::GetValues() {
	return m_values;
}

boost::multi_array_ref<
	std::unique_ptr<boost::multi_array_ref<float, 1>>,
1> EvaluationBatch::GetPolicies() {
	return m_policies;
}

size_t EvaluationBatch::GetSize() const {
	return m_size;
}

size_t EvaluationBatch::GetElementSize() const {
	return m_element_size;
}

tensorflow::Tensor& EvaluationBatch::GetBatchTensor() {
	return m_batch;
}


float* EvaluationBatch::GetValue(size_t index) {
	return m_values[index];
}


boost::multi_array_ref<float, 1> EvaluationBatch::GetPolicy(size_t index) {
	return *(m_policies[index]);
}


void EvaluationBatch::Lock() {
	m_lock.Lock();
}


void EvaluationBatch::Unlock() {
	m_lock.Unlock();
}


bool EvaluationBatch::IsFull() {
	return m_current_index >= GetSize();
}


size_t EvaluationBatch::GetNumberOfElements() const {
	return m_current_index;
}


oaz::thread_pool::Task* EvaluationBatch::GetTask(size_t index) {
	return m_tasks[index];
}


NNEvaluator::NNEvaluator(
	std::shared_ptr<Model> model, 
	std::shared_ptr<oaz::cache::Cache> cache,
	std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
	const std::vector<int>& element_dimensions,
	size_t batch_size): 
	m_batch_size(batch_size), 
	m_model(model),
	m_cache(cache),
	m_n_evaluation_requests(0),
	m_n_evaluations(0),
	m_thread_pool(thread_pool),
	m_element_dimensions(element_dimensions) {
		std::future<void> future_exit_signal = m_exit_signal.get_future();
		m_worker = std::thread(	
			&NNEvaluator::Monitor,
			this,
			std::move(future_exit_signal)
		);
}


NNEvaluator::~NNEvaluator() {
	m_exit_signal.set_value();
	m_worker.join();
}


void NNEvaluator::Monitor(std::future<void> future_exit_signal) {
	while(future_exit_signal.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout) {
		if(!m_evaluation_completed) {
			ForceEvaluation();
		}
		m_evaluation_completed = false;
	}
};


void NNEvaluator::AddNewBatch() {
	std::unique_ptr<EvaluationBatch> batch = std::make_unique<EvaluationBatch>(
		GetElementDimensions(), GetBatchSize()
	);
	m_batches.push_back(std::move(batch));
}

const std::vector<int>& NNEvaluator::GetElementDimensions() const {
	return m_element_dimensions;
}

size_t NNEvaluator::GetBatchSize() const {
	return m_batch_size;
}

std::string NNEvaluator::GetStatus() const {
	return "Evaluator status: " + std::to_string(m_n_evaluations * m_batch_size) + "/" + std::to_string(m_n_evaluation_requests * m_batch_size);
}


void NNEvaluator::RequestEvaluation(
	oaz::games::Game* game, 
	float* value,
	boost::multi_array_ref<float, 1> policy,
	oaz::thread_pool::Task* task
	) {
	if(m_cache)
		if(EvaluateFromCache(game, value, policy, task))
			return;
	EvaluateFromNN(game, value, policy, task);
}

bool NNEvaluator::EvaluateFromCache(
	oaz::games::Game* game, 
	float* value,
	boost::multi_array_ref<float, 1> policy,
	oaz::thread_pool::Task* task
	) {
		bool success = m_cache->Evaluate(*game, *value, policy);	
		if(success)
			m_thread_pool->enqueue(task);
		return success;
}

void NNEvaluator::EvaluateFromNN(
	oaz::games::Game* game, 
	float* value,
	boost::multi_array_ref<float, 1> policy,
	oaz::thread_pool::Task* task
	) {
	m_batches.Lock();
	if(!m_batches.empty()) {
		EvaluationBatch* current_batch = m_batches.back().get();
		current_batch->Lock();
		if(!current_batch->IsFull()) {
			size_t index = current_batch->AcquireIndex();
			current_batch->Unlock();
			m_batches.Unlock();
		
			current_batch->InitialiseElement(
				index,
				game,
				value,
				policy,
				task
			);
		}
		else {
			std::unique_ptr<EvaluationBatch> current_batch_uniqueptr = std::move(m_batches.back());
			m_batches.pop_back();
			AddNewBatch();
			current_batch->Unlock();
			m_batches.Unlock();
		
			while(!current_batch->IsAvailableForEvaluation());

			EvaluateBatch(current_batch);

			EvaluateFromNN(
				game, 
				value,
				policy,
				task
			);
		}
	} else {
		AddNewBatch();
		m_batches.Unlock();
		EvaluateFromNN(
			game, 
			value,
			policy,
			task
		);
	}
}


void NNEvaluator::EvaluateBatch(EvaluationBatch* batch) {

	/* spdlog::debug("Evaluating batch of size {}", batch->getNElements()); */
	std::vector<tensorflow::Tensor> outputs;

	m_n_evaluation_requests++;
	m_model->Run(
		{{"input:0", batch->GetBatchTensor()}}, 
		{m_model->GetValueNodeName(), m_model->GetPolicyNodeName()},
		{},
		&outputs
	);
	m_n_evaluations++;

 	auto values_map = outputs[0].template tensor<float, 2>();
 	auto policies_map = outputs[1].template tensor<float, 2>();

	for(size_t i=0; i != batch->GetNumberOfElements(); ++i) {
		std::memcpy(
			batch->GetValue(i), 
			&values_map(i, 0),
			1 * sizeof(float)
		);

		auto policy = batch->GetPolicy(i);
		std::memcpy(
			policy.origin(),
			&policies_map(i, 0),
			policy.num_elements() * sizeof(float)
		);
	}
	
	if(m_cache)
		m_cache->BatchInsert(
			batch->GetGames(),
			batch->GetValues(),
			batch->GetPolicies(),
			batch->GetNumberOfElements()
		);

	for(size_t i=0; i!= batch->GetNumberOfElements(); ++i)
		m_thread_pool->enqueue(batch->GetTask(i));

}


void NNEvaluator::ForceEvaluation() {

	/* spdlog::debug("Forced evaluation"); */
	m_batches.Lock();
	if (!m_batches.empty()) {
		EvaluationBatch* earliest_batch = m_batches.front().get();
		earliest_batch->Lock();
		if(earliest_batch->IsAvailableForEvaluation()) {
			std::unique_ptr<EvaluationBatch> earliest_batch_uptr = std::move(m_batches.front());
			m_batches.pop_front();
			earliest_batch->Unlock();
			m_batches.Unlock();
			EvaluateBatch(earliest_batch_uptr.get());
		}
		else {
			earliest_batch->Unlock();
			m_batches.Unlock();
		}
	} else m_batches.Unlock();
}
