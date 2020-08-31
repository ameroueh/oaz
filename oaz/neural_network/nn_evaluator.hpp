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
#include "oaz/evaluator/evaluator.hpp"
#include "boost/multi_array.hpp"

#include "oaz/neural_network/model.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::nn {
	template <class Game>	
	class EvaluationBatch {
		
		TEST_FRIENDS;

		public:
			EvaluationBatch(size_t size);
			void readElementFromMemory(
				size_t, 
				float*, 
				typename Game::Value*,
				typename Game::Policy*,
				oaz::thread_pool::Task*
			);
			bool availableForEvaluation();
			size_t acquireIndex();
			size_t getSize() const;
			tensorflow::Tensor& getBatchTensor();
			typename Game::Value* getValue(size_t);			
			typename Game::Policy* getPolicy(size_t);
			void lock();			
			void unlock();
			bool full();
			size_t getNElements() const;
			oaz::thread_pool::Task* getTask(size_t);
		private:
			oaz::mutex::SpinlockMutex m_lock;
			tensorflow::Tensor m_batch;
			boost::multi_array<oaz::thread_pool::Task*, 1> m_tasks;
			boost::multi_array<typename Game::Value*, 1> m_values;
			boost::multi_array<typename Game::Policy*, 1> m_policies;
			size_t m_current_index;
			size_t m_size;
			std::atomic<size_t> m_n_reads;

	};

	template <class Game> 
	class NNEvaluator : public oaz::evaluator::Evaluator<Game> { 

		TEST_FRIENDS;

		public: 
			using SharedModelPointer = std::shared_ptr<Model>;
			
			NNEvaluator(SharedModelPointer, oaz::thread_pool::ThreadPool*, size_t);
			~NNEvaluator();
			
			void requestEvaluation(
				Game*, 
				typename Game::Value*,
				typename Game::Policy*,
				oaz::thread_pool::Task*
			);
			void forceEvaluation();

			std::string getStatus() const;
		private:
			using Batch = EvaluationBatch<Game>;
			using UniqueBatchPointer = std::unique_ptr<Batch>;
			
			void evaluateBatch(Batch*);
			void addNewBatch();
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
			oaz::thread_pool::ThreadPool* m_thread_pool;
	};
}

#include "nn_evaluator_impl.cpp"
#endif // __NN_EVALUATOR_H__
