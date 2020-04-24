#ifndef __NN_TRAINER_H__
#define __NN_TRAINER_H__

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/saved_model/loader.h"

namespace oaz::nn {
	
	template <class Game>
	class TrainingBatch {
		
		public:
			TrainingBatch(size_t size): 
				m_current_index(0),
				m_n_reads(0),
				m_size(size) {
					std::vector<long long int> dimensions = Game::getBoardDimensions();
					m_board_size_bytes = std::accumulate(
						dimensions.begin(), 
						dimensions.end(), 
						1,
						std::multiplies<long long int>()
					) * sizeof(float);
					
					m_value_size_bytes = sizeof(float);
					m_policy_size_bytes = Game::getPolicySize() * sizeof(float);
					
					std::vector<long long int> tensor_dimensions = {(long long int) size};
					tensor_dimensions.insert(tensor_dimensions.end(), dimensions.begin(), dimensions.end());
					m_board_batch = tensorflow::Tensor(
						tensorflow::DT_FLOAT,
						tensorflow::TensorShape(tensor_dimensions)
					);
					
					m_value_label_batch = tensorflow::Tensor(
						tensorflow::DT_FLOAT,
						tensorflow::TensorShape(
						 {(long long int) size}
						)
					);

					m_policy_label_batch = tensorflow::Tensor(
						tensorflow::DT_FLOAT,
						tensorflow::TensorShape(
						 {(long long int) size, (long long int) Game::getPolicySize()})
					);
			}
			
			void readElementFromMemory(
				size_t index, 
				float* board_source,
				float* value_label_source,
				float* policy_label_source) {
				float* board_destination = &m_board_batch.template tensor<float, Game::NBoardDimensions + 1>()(index, 0, 0, 0);
				float* policy_label_destination = &m_policy_label_batch.template tensor<float, 2>()(index, 0);
				float* value_label_destination = &m_value_label_batch.template tensor<float, 1>()(index);

				std::memcpy(board_destination, board_source, m_board_size_bytes);
				std::memcpy(value_label_destination, value_label_source, m_value_size_bytes);
				std::memcpy(policy_label_destination, policy_label_source, m_policy_size_bytes);
				++m_n_reads;
			}

			bool availableForTraining() {
				return (m_current_index == m_n_reads) && full();
			}

			size_t acquireIndex() {
				size_t index = m_current_index;
				++m_current_index;
				return	index;
			}

			size_t getSize() const {
				return m_size;
			}
			
			tensorflow::Tensor& getBoardTensor() {
				return m_board_batch;
			}
			
			tensorflow::Tensor& getValueLabelTensor() {
				return m_value_label_batch;
			}
			
			tensorflow::Tensor& getPolicyLabelTensor() {
				return m_policy_label_batch;
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

		private:
			oaz::mutex::SpinlockMutex m_lock;
			
			size_t m_current_index;
			size_t m_size;
			size_t m_board_size_bytes;
			size_t m_value_size_bytes;
			size_t m_policy_size_bytes;
			std::atomic<size_t> m_n_reads;
			
			tensorflow::Tensor m_board_batch;
			tensorflow::Tensor m_policy_label_batch;
			tensorflow::Tensor m_value_label_batch;
	};

	template <class Game>
	class NNTrainer {
		public:
			NNTrainer(size_t, size_t);
			void load_model(std::string);
			void addTrainingExample(
				typename Game::Board*,
				typename Game::Value*,
				typename Game::Policy*
			);
			
			void checkpoint(std::string);

		private:
			using Batch = TrainingBatch<Game>;
			using UniqueBatchPointer = std::unique_ptr<Batch>;
			using UniqueSessionPointer = std::unique_ptr<tensorflow::Session>;
			
			oaz::queue::SafeDeque<UniqueBatchPointer> m_batches;
			
			void initialise();
			void trainFromBatch(Batch*);
			void maybeTrain();
			void addNewBatch();
			size_t getEpochSize() const;
			
			UniqueSessionPointer m_session;
			size_t m_batch_size;
			size_t m_epoch_size;
	};
}

#include "nn_trainer_impl.cpp"
#endif
