#ifndef __NN_EVALUATOR_H__
#define __NN_EVALUATOR_H__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <future>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"

#include "oaz/queue/queue.hpp"
#include "oaz/mutex/mutex.hpp"
#include "boost/multi_array.hpp"

#include "oaz/neural_network/model.hpp"

namespace oaz::nn {
	template <class Game, class Notifier>	
	class EvaluationBatch {
		
		TEST_FRIENDS;

		public:
			EvaluationBatch(size_t size): 
				m_current_index(0),
				m_n_reads(0),
				m_size(size),
				m_values(boost::extents[size]),
				m_policies(boost::extents[size]),
				m_notifiers(boost::extents[size]) {

					auto dimensions = Game::Board::Dimensions();
					
					std::vector<long long int> tensor_dimensions = {(long long int) size};
					tensor_dimensions.insert(tensor_dimensions.end(), dimensions.begin(), dimensions.end());
					m_batch = tensorflow::Tensor(
						tensorflow::DT_FLOAT,
						tensorflow::TensorShape(tensor_dimensions)
					);
			}
			
			void readElementFromMemory(
				size_t index, 
				float* source,
				typename Game::Value* value,
				typename Game::Policy* policy,
				Notifier notifier) {
				float* destination = m_batch.template tensor<float, Game::Board::NumDimensions() + 1>().data() + index * Game::Board::NumElements();
				std::memcpy(destination, source, Game::Board::SizeBytes());
				m_values[index] = value;
				m_policies[index] = policy;
				m_notifiers[index] = notifier;
				++m_n_reads;
			}

			bool availableForEvaluation() {
				return m_current_index == m_n_reads;
			}

			size_t acquireIndex() {
				size_t index = m_current_index;
				++m_current_index;
				return	index;
			}

			size_t getSize() const {
				return m_size;
			}

			tensorflow::Tensor& getBatchTensor() {
				return m_batch;
			}

			typename Game::Value* getValue(size_t index) {
				return m_values[index];
			}
			
			typename Game::Policy* getPolicy(size_t index) {
				return m_policies[index];
			}

			void lock() {
				m_lock.lock();
			}
			
			void unlock() {
				m_lock.unlock();
			}

			bool full() {
				return m_current_index >= getSize();
			}

			size_t getNElements() const {
				return m_current_index;
			}

			Notifier& getNotifier(size_t index) {
				return m_notifiers[index];
			}
		
		private:
			oaz::mutex::SpinlockMutex m_lock;
			tensorflow::Tensor m_batch;
			boost::multi_array<Notifier, 1> m_notifiers;
			boost::multi_array<typename Game::Value*, 1> m_values;
			boost::multi_array<typename Game::Policy*, 1> m_policies;
			size_t m_current_index;
			size_t m_size;
			std::atomic<size_t> m_n_reads;

	};

	template <class Game, class Notifier> 
	class NNEvaluator { 
		public: 
			using SharedModelPointer = std::shared_ptr<Model>;
			
			NNEvaluator(SharedModelPointer, size_t);
			~NNEvaluator();
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				Notifier
			);
			void addNewBatch();
			void forceEvaluation();

			std::string getStatus() const;
		private:
			using Batch = EvaluationBatch<Game, Notifier>;
			using UniqueBatchPointer = std::unique_ptr<Batch>;
			
			void evaluateBatch(Batch*);

			void monitor(std::future<void>);
			
			oaz::queue::SafeDeque<UniqueBatchPointer> m_batches;
			oaz::mutex::SpinlockMutex m_requests_lock;
			SharedModelPointer m_model;
			size_t m_batch_size;

			std::atomic<size_t> m_n_evaluation_requests;
			std::atomic<size_t> m_n_evaluations;

			std::thread m_worker;
			std::promise<void> m_exit_signal;

			std::atomic<bool> m_evaluation_completed;
	};
}

#include "nn_evaluator_impl.cpp"
#endif // __NN_EVALUATOR_H__
