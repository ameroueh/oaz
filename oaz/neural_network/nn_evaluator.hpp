#ifndef OAZ_NEURAL_NETWORK_NN_EVALUATOR_HPP_
#define OAZ_NEURAL_NETWORK_NN_EVALUATOR_HPP_

#ifndef TEST_FRIENDS
  #define TEST_FRIENDS
#endif

#include <stdint.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/cache/cache.hpp"
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/mutex/mutex.hpp"
#include "oaz/neural_network/model.hpp"
#include "oaz/queue/queue.hpp"
#include "oaz/thread_pool/thread_pool.hpp"
#include "tensorflow/core/framework/tensor.h"

namespace oaz::nn {

class EvaluationBatchStatistics {
 public:
  EvaluationBatchStatistics()
      : evaluation_forced(false),
        n_elements(0),
        size(0),
        time_created(0),
        time_evaluation_start(0),
        time_evaluation_end(0) {}

  size_t time_created;
  size_t time_evaluation_start;
  ssize_t time_evaluation_end;
  size_t size;
  size_t n_elements;
  bool evaluation_forced;
};

class EvaluationBatch {
  TEST_FRIENDS;

 public:
  EvaluationBatch(const std::vector<int>&, size_t);
  bool IsAvailableForEvaluation() const;
  size_t GetSize() const;
  size_t GetElementSize() const;
  float* GetValue(size_t);

  size_t AcquireIndex();
  void InitialiseElement(size_t, oaz::games::Game*, float*,
                         const boost::multi_array_ref<float, 1>&,
                         oaz::thread_pool::Task*);

  tensorflow::Tensor& GetBatchTensor();
  boost::multi_array_ref<float, 1> GetPolicy(size_t);
  size_t GetNumberOfElements() const;
  oaz::thread_pool::Task* GetTask(size_t);
  void Lock();
  void Unlock();
  bool IsFull() const;

  boost::multi_array_ref<oaz::games::Game*, 1> GetGames();
  boost::multi_array_ref<float*, 1> GetValues();
  boost::multi_array_ref<std::unique_ptr<boost::multi_array_ref<float, 1>>, 1>
  GetPolicies();

  EvaluationBatchStatistics& GetStatistics();

 private:
  oaz::mutex::SpinlockMutex m_lock;
  tensorflow::Tensor m_batch;
  boost::multi_array<oaz::thread_pool::Task*, 1> m_tasks;
  boost::multi_array<oaz::games::Game*, 1> m_games;
  boost::multi_array<float*, 1> m_values;
  boost::multi_array<std::unique_ptr<boost::multi_array_ref<float, 1>>, 1>
      m_policies;
  size_t m_current_index;
  size_t m_size;
  size_t m_element_size;
  std::atomic<size_t> m_n_reads;

  std::unique_ptr<EvaluationBatchStatistics> m_statistics;
};

class NNEvaluator : public oaz::evaluator::Evaluator {
  TEST_FRIENDS;

 public:
  NNEvaluator(std::shared_ptr<oaz::nn::Model>,
              std::shared_ptr<oaz::cache::Cache>,
              std::shared_ptr<oaz::thread_pool::ThreadPool>,
              const std::vector<int>&, size_t);
  void RequestEvaluation(oaz::games::Game* game, float* value,
                         boost::multi_array_ref<float, 1> policy,
                         oaz::thread_pool::Task* task) override;

  std::vector<EvaluationBatchStatistics> GetStatistics();

  ~NNEvaluator();
  NNEvaluator(const NNEvaluator&) = delete;
  NNEvaluator(NNEvaluator&&) = delete;
  NNEvaluator& operator=(const NNEvaluator&) = delete;
  NNEvaluator& operator=(NNEvaluator&&) = delete;

 private:
  static constexpr size_t WAIT_BEFORE_FORCED_EVAL_MS = 10;
  bool EvaluateFromCache(oaz::games::Game*, float*,
                         const boost::multi_array_ref<float, 1>&,
                         oaz::thread_pool::Task*);
  void EvaluateFromNN(oaz::games::Game*, float*,
                      const boost::multi_array_ref<float, 1>&,
                      oaz::thread_pool::Task*);

  size_t GetBatchSize() const;
  const std::vector<int>& GetElementDimensions() const;
  void ForceEvaluation();
  void EvaluateBatch(EvaluationBatch*);
  void AddNewBatch();
  void Monitor(std::future<void>);
  void StartMonitor();
  void ArchiveBatchStatistics(const EvaluationBatchStatistics&);

  oaz::queue::SafeDeque<std::shared_ptr<EvaluationBatch>> m_batches;
  oaz::mutex::SpinlockMutex m_requests_lock;

  size_t m_batch_size;

  std::atomic<size_t> m_n_evaluation_requests;
  std::atomic<size_t> m_n_evaluations;

  std::thread m_worker;
  std::promise<void> m_exit_signal;
  std::vector<int> m_element_dimensions;

  std::shared_ptr<Model> m_model;
  std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
  std::shared_ptr<oaz::cache::Cache> m_cache;

  oaz::mutex::SpinlockMutex m_archive_lock;
  std::vector<EvaluationBatchStatistics> m_archive;
};
}  // namespace oaz::nn

#endif  // OAZ_NEURAL_NETWORK_NN_EVALUATOR_HPP_
