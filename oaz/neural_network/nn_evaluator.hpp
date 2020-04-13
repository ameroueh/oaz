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

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/saved_model/loader.h"

#include "oaz/queue/queue.hpp"
#include "boost/multi_array.hpp"

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
					std::initializer_list<long long int> dimensions = Game::getBoardDimensions();
					m_element_size_bytes = std::accumulate(
						dimensions.begin(), 
						dimensions.end(), 
						1,
						std::multiplies<size_t>()
					) * sizeof(float);
					
					std::vector<long long int> tensor_dimensions = {(long long int) size};
					tensor_dimensions.insert(tensor_dimensions.end(), dimensions);
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
				float* destination = &m_batch.template tensor<float, Game::NBoardDimensions + 1>()(index, 0, 0, 0);
				std::memcpy(destination, source, m_element_size_bytes);
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
			size_t m_element_size_bytes;
			std::atomic<size_t> m_n_reads;

	};

	template <class Game, class Notifier> 
	class NNEvaluator { 
		public: 
			NNEvaluator(size_t);
			void load_model(std::string);
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				Notifier
			);
			void forceEvaluation();
			void addNewBatch();
		private:
			using Batch = EvaluationBatch<Game, Notifier>;
			using UniqueBatchPointer = std::unique_ptr<Batch>;
			using UniqueSessionPointer = std::unique_ptr<tensorflow::Session>;
			
			void initialise();
			void evaluateBatch(Batch*);
			
			oaz::queue::SafeDeque<UniqueBatchPointer> m_batches;
			oaz::mutex::SpinlockMutex m_requests_lock;
			UniqueSessionPointer m_session;
			size_t m_batch_size;
	};
}

#include "nn_evaluator_impl.cpp"
#endif // __NN_EVALUATOR_H__
