#include "nn_trainer.hpp"

#include <queue>
#include <iostream>

template <class Game>
NNTrainer<Game>::NNTrainer(SharedModelPointer model, size_t batch_size, size_t epoch_size): 
	m_batch_size(batch_size),
	m_epoch_size(epoch_size),
	m_model(model) {}

template <class Game>
size_t NNTrainer<Game>::getEpochSize() const {
	return m_epoch_size;
}

template <class Game>
void NNTrainer<Game>::addTrainingExample(
	typename Game::Board* board,
	typename Game::Value* value_label,
	typename Game::Policy* policy_label) {
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
				&(*board)[0][0][0],
				value_label,
				&(*policy_label)[0]
			);

		}
		else {
			current_batch->unlock();
			addNewBatch();
			m_batches.unlock();
			
			addTrainingExample(
				board,
				value_label,
				policy_label
			);
		} 
	} else {
		addNewBatch();
		m_batches.unlock();
		addTrainingExample(
			board,
			value_label,
			policy_label
		);
	}
	
	maybeTrain();
}

template <class Game>
void NNTrainer<Game>::trainFromBatch(Batch* batch) {

	std::vector<tensorflow::Tensor> outputs;
	for(size_t i=0; i!=batch->getSize(); ++i) 
	m_model->Run(
		{{"input:0", batch->getBoardTensor()}, 
		 {"value_labels:0", batch->getValueLabelTensor()}, 
		 {"policy_labels:0", batch->getPolicyLabelTensor()}},
		{"loss"},
		{"train"},
		&outputs
	);
	std::cout << "Training batch loss: " << *(outputs[0].flat<float>().data()) << std::endl;
}

template <class Game>
void NNTrainer<Game>::maybeTrain() {
	m_batches.lock();
	if(m_batches.size() >= getEpochSize()) {
		std::queue<UniqueBatchPointer> batches_for_training;
		while(!m_batches.empty()) {
			if(!m_batches.front()->availableForTraining())
				break;
			batches_for_training.push(std::move(m_batches.front()));
			m_batches.pop_front();
		}
		m_batches.unlock();
		while(!batches_for_training.empty()) {
			UniqueBatchPointer batch_for_training = std::move(batches_for_training.front());
			batches_for_training.pop();
			trainFromBatch(batch_for_training.get());
		}
	} else m_batches.unlock();
}
	
template <class Game>
void NNTrainer<Game>::addNewBatch() {
	UniqueBatchPointer batch(new Batch(m_batch_size));
	m_batches.push_back(std::move(batch));
}
