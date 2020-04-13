#include <string>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/protobuf/meta_graph.pb.h"

#include "oaz/neural_network/nn_evaluator.hpp"

#include <iostream>
#include <queue>
#include <thread>
#include <memory>

using namespace oaz::nn;
using namespace tensorflow;
using namespace std;

template <class Game, class Notifier>
NNEvaluator<Game, Notifier>::NNEvaluator(size_t batch_size): 
	m_batch_size(batch_size), 
	m_session(nullptr) {
	initialise();
}

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::addNewBatch() {
	UniqueBatchPointer batch(new Batch(m_batch_size));
	m_batches.push_back(std::move(batch));
}


template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::initialise() {
	tensorflow::SessionOptions options;
	tensorflow::Session* session;
  	TF_CHECK_OK(tensorflow::NewSession(options, &session));
	m_session.reset(session);
	addNewBatch();
}

template <class Game, class Notifier>
void NNEvaluator<Game, Notifier>::load_model(std::string model_path) {
	
	MetaGraphDef graph_def;
	ReadBinaryProto(Env::Default(), model_path + "/graph.pb", &graph_def);
	m_session->Create(graph_def.graph_def());
	
	Tensor checkpoint_path_tensor(DT_STRING, TensorShape());
	checkpoint_path_tensor.scalar<std::string>()() = model_path + "/model";
	m_session->Run(
		{{graph_def.saver_def().filename_tensor_name(), checkpoint_path_tensor},},
		{},
		{graph_def.saver_def().restore_op_name()},
		nullptr
	);
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
				&(game->getBoard()[0][0][0]),
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
	TF_CHECK_OK(m_session->Run(
		{{"input:0", batch->getBatchTensor()}}, 
		{"value", "policy"},
		{},
		&outputs
	));

 	auto values_map = outputs[0].template tensor<float, 1>();
 	auto policies_map = outputs[1].template tensor<float, 2>();

	for(size_t i=0; i != batch->getNElements(); ++i) {
		std::memcpy(
			batch->getValue(i), 
			&values_map(i),
			1 * sizeof(float)
		);

		std::memcpy(
			batch->getPolicy(i), 
			&policies_map(i, 0),
			Game::getPolicySize() * sizeof(float)
				
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
