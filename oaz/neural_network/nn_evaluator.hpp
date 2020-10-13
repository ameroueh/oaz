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
	class EvaluationBatch {
		
		TEST_FRIENDS;

		public:
			EvaluationBatch(
				const std::vector<int>&,
				size_t
			);
			bool IsAvailableForEvaluation() const;
			size_t GetSize() const;
			size_t GetElementSize() const;
			float* GetValue(size_t);			
			
			size_t AcquireIndex();
			void InitialiseElement(
				size_t, 
				oaz::games::Game*, 
				float*,
				boost::multi_array_ref<float, 1>,
				oaz::thread_pool::Task*
			);
			
			tensorflow::Tensor& GetBatchTensor();
			boost::multi_array_ref<float, 1> GetPolicy(size_t);
			size_t GetNumberOfElements() const;
			oaz::thread_pool::Task* GetTask(size_t);
			void Lock();			
			void Unlock();
			bool IsFull();
		private:
			oaz::mutex::SpinlockMutex m_lock;
			tensorflow::Tensor m_batch;
			boost::multi_array<oaz::thread_pool::Task*, 1> m_tasks;
			boost::multi_array<float*, 1> m_values;
			boost::multi_array<
				std::unique_ptr<
					boost::multi_array_ref<float, 1>
				>,
				1
			> m_policies;
			size_t m_current_index;
			size_t m_size;
			size_t m_element_size;
			std::atomic<size_t> m_n_reads;

	};

	class NNEvaluator : public oaz::evaluator::Evaluator { 

		TEST_FRIENDS;

		public: 
			NNEvaluator(
				std::shared_ptr<Model>, 
				std::shared_ptr<oaz::thread_pool::ThreadPool>,
				const std::vector<int>&,
				size_t
			);
			void RequestEvaluation(
				oaz::games::Game*, 
				float*,
				boost::multi_array_ref<float, 1>,
				oaz::thread_pool::Task*
			);

			std::string GetStatus() const;
			~NNEvaluator();
		private:
			size_t GetBatchSize() const;
			const std::vector<int>& GetElementDimensions() const;
			void ForceEvaluation();
			void EvaluateBatch(EvaluationBatch*);
			void AddNewBatch();
			void Monitor(std::future<void>);
			
			oaz::queue::SafeDeque<std::unique_ptr<EvaluationBatch>> m_batches;
			oaz::mutex::SpinlockMutex m_requests_lock;

			size_t m_batch_size;

			std::atomic<size_t> m_n_evaluation_requests;
			std::atomic<size_t> m_n_evaluations;

			std::thread m_worker;
			std::promise<void> m_exit_signal;
			std::atomic<bool> m_evaluation_completed;


			std::vector<int> m_element_dimensions;
			std::shared_ptr<Model> m_model;
			std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
	};
}

#include "nn_evaluator_impl.cpp"
#endif // __NN_EVALUATOR_H__
