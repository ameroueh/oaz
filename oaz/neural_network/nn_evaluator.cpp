#include "oaz/neural_network/nn_evaluator.hpp"

#include <stdint.h>

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include "boost/multi_array.hpp"
#include "oaz/utils/time.hpp"
#include "tensorflow/core/framework/tensor.h"

oaz::nn::EvaluationBatch::EvaluationBatch(
    const std::vector<int>& element_dimensions, size_t size)
    : m_current_index(0),
      m_n_reads(0),
      m_size(size),
      m_element_size(std::accumulate(element_dimensions.cbegin(),
                                     element_dimensions.cend(), 1,
                                     std::multiplies<int>())),
      m_games(boost::extents[size]),
      m_values(boost::extents[size]),
      m_policies(boost::extents[size]),
      m_tasks(boost::extents[size]),
      m_statistics(std::make_unique<oaz::nn::EvaluationBatchStatistics>()) {
  std::vector<tensorflow::int64> tensor_dimensions = {
      static_cast<tensorflow::int64>(size)};
  tensor_dimensions.insert(tensor_dimensions.end(), element_dimensions.begin(),
                           element_dimensions.end());
  m_batch = tensorflow::Tensor(tensorflow::DT_FLOAT,
                               tensorflow::TensorShape(tensor_dimensions));

  GetStatistics().time_created = oaz::utils::time_now_ns();
  GetStatistics().size = GetSize();
}

oaz::nn::EvaluationBatchStatistics& oaz::nn::EvaluationBatch::GetStatistics() {
  return *m_statistics;
}

void oaz::nn::EvaluationBatch::InitialiseElement(
    size_t index, oaz::games::Game* game, float* value,
    const boost::multi_array_ref<float, 1>& policy,
    oaz::thread_pool::Task* task) {
  float* destination = m_batch.flat<float>().data() + index * GetElementSize();
  game->WriteCanonicalStateToTensorMemory(destination);
  m_games[index] = game;
  m_values[index] = value;
  m_policies[index] =
      std::move(std::make_unique<boost::multi_array_ref<float, 1>>(policy));
  m_tasks[index] = task;
  ++m_n_reads;
}

bool oaz::nn::EvaluationBatch::IsAvailableForEvaluation() const {
  return m_current_index == m_n_reads;
}

size_t oaz::nn::EvaluationBatch::AcquireIndex() {
  size_t index = m_current_index;
  ++m_current_index;
  return index;
}

boost::multi_array_ref<oaz::games::Game*, 1>
oaz::nn::EvaluationBatch::GetGames() {
  return boost::multi_array_ref<oaz::games::Game*, 1>(
      m_games.origin(), boost::extents[GetSize()]);
}

boost::multi_array_ref<float*, 1> oaz::nn::EvaluationBatch::GetValues() {
  return boost::multi_array_ref<float*, 1>(m_values.origin(),
                                           boost::extents[GetSize()]);
}

boost::multi_array_ref<std::unique_ptr<boost::multi_array_ref<float, 1>>, 1>
oaz::nn::EvaluationBatch::GetPolicies() {
  return boost::multi_array_ref<
      std::unique_ptr<boost::multi_array_ref<float, 1>>, 1>(
      m_policies.origin(), boost::extents[GetSize()]);
}
size_t oaz::nn::EvaluationBatch::GetSize() const { return m_size; }

size_t oaz::nn::EvaluationBatch::GetElementSize() const {
  return m_element_size;
}

tensorflow::Tensor& oaz::nn::EvaluationBatch::GetBatchTensor() {
  return m_batch;
}

float* oaz::nn::EvaluationBatch::GetValue(size_t index) {
  return m_values[index];
}

boost::multi_array_ref<float, 1> oaz::nn::EvaluationBatch::GetPolicy(
    size_t index) {
  return *(m_policies[index]);
}

void oaz::nn::EvaluationBatch::Lock() { m_lock.Lock(); }

void oaz::nn::EvaluationBatch::Unlock() { m_lock.Unlock(); }

bool oaz::nn::EvaluationBatch::IsFull() const {
  return m_current_index >= GetSize();
}

size_t oaz::nn::EvaluationBatch::GetNumberOfElements() const {
  return m_current_index;
}

oaz::thread_pool::Task* oaz::nn::EvaluationBatch::GetTask(size_t index) {
  return m_tasks[index];
}

oaz::nn::NNEvaluator::NNEvaluator(
    std::shared_ptr<Model> model, std::shared_ptr<oaz::cache::Cache> cache,
    std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
    const std::vector<int>& element_dimensions, size_t batch_size)
    : m_batch_size(batch_size),
      m_model(std::move(model)),
      m_cache(std::move(cache)),
      m_n_evaluation_requests(0),
      m_n_evaluations(0),
      m_thread_pool(std::move(thread_pool)),
      m_element_dimensions(element_dimensions) {
  StartMonitor();
}

oaz::nn::NNEvaluator::~NNEvaluator() {
  m_exit_signal.set_value();
  m_worker.join();
}

void oaz::nn::NNEvaluator::Monitor(std::future<void> future_exit_signal) {
  while (future_exit_signal.wait_for(std::chrono::milliseconds(
             WAIT_BEFORE_FORCED_EVAL_MS)) == std::future_status::timeout) {
    ForceEvaluation();
  }
}

void oaz::nn::NNEvaluator::StartMonitor() {
  std::future<void> future_exit_signal = m_exit_signal.get_future();
  m_worker = std::thread(&oaz::nn::NNEvaluator::Monitor, this,
                         std::move(future_exit_signal));
}

void oaz::nn::NNEvaluator::AddNewBatch() {
  std::shared_ptr<oaz::nn::EvaluationBatch> batch =
      std::make_shared<oaz::nn::EvaluationBatch>(GetElementDimensions(),
                                                 GetBatchSize());
  m_batches.push_back(std::move(batch));
}

const std::vector<int>& oaz::nn::NNEvaluator::GetElementDimensions() const {
  return m_element_dimensions;
}

size_t oaz::nn::NNEvaluator::GetBatchSize() const { return m_batch_size; }

void oaz::nn::NNEvaluator::RequestEvaluation(
    oaz::games::Game* game, float* value,
    boost::multi_array_ref<float, 1> policy, oaz::thread_pool::Task* task) {
  if (m_cache && EvaluateFromCache(game, value, policy, task)) {
    return;
  }
  EvaluateFromNN(game, value, policy, task);
}

bool oaz::nn::NNEvaluator::EvaluateFromCache(
    oaz::games::Game* game, float* value,
    const boost::multi_array_ref<float, 1>& policy,
    oaz::thread_pool::Task* task) {
  bool success = m_cache->Evaluate(*game, value, policy);
  if (success) {
    m_thread_pool->enqueue(task);
  }
  return success;
}

void oaz::nn::NNEvaluator::EvaluateFromNN(
    oaz::games::Game* game, float* value,
    const boost::multi_array_ref<float, 1>& policy,
    oaz::thread_pool::Task* task) {
  m_batches.Lock();
  if (!m_batches.empty()) {
    auto current_batch = m_batches.back();
    current_batch->Lock();

    size_t index = current_batch->AcquireIndex();

    bool evaluate_batch = current_batch->IsFull();
    if (evaluate_batch) {
      m_batches.pop_back();
    }

    current_batch->Unlock();
    m_batches.Unlock();

    current_batch->InitialiseElement(index, game, value, policy, task);

    if (evaluate_batch) {
      while (!current_batch->IsAvailableForEvaluation()) {
      }
      EvaluateBatch(current_batch.get());
    }

  } else {
    AddNewBatch();
    m_batches.Unlock();

    EvaluateFromNN(game, value, policy, task);
  }
}

void oaz::nn::NNEvaluator::EvaluateBatch(oaz::nn::EvaluationBatch* batch) {
  batch->GetStatistics().time_evaluation_start = oaz::utils::time_now_ns();
  batch->GetStatistics().n_elements = batch->GetNumberOfElements();

  std::vector<tensorflow::Tensor> outputs;

  m_n_evaluation_requests++;
  m_model->Run(
      {{m_model->GetInputNodeName(),
        batch->GetBatchTensor().Slice(0, batch->GetNumberOfElements())}},
      {m_model->GetValueNodeName(), m_model->GetPolicyNodeName()}, {},
      &outputs);
  m_n_evaluations++;

  auto values_map = outputs[0].template tensor<float, 2>();
  auto policies_map = outputs[1].template tensor<float, 2>();

  for (size_t i = 0; i != batch->GetNumberOfElements(); ++i) {
    std::memcpy(batch->GetValue(i), &values_map(i, 0), 1 * sizeof(float));

    auto policy = batch->GetPolicy(i);
    std::memcpy(policy.origin(), &policies_map(i, 0),
                policy.num_elements() * sizeof(float));
  }

  if (m_cache) {
    m_cache->BatchInsert(batch->GetGames(), batch->GetValues(),
                         batch->GetPolicies(), batch->GetNumberOfElements());
  }

  for (size_t i = 0; i != batch->GetNumberOfElements(); ++i) {
    m_thread_pool->enqueue(batch->GetTask(i));
  }

  batch->GetStatistics().time_evaluation_end = oaz::utils::time_now_ns();

  ArchiveBatchStatistics(batch->GetStatistics());
}

void oaz::nn::NNEvaluator::ArchiveBatchStatistics(
    const oaz::nn::EvaluationBatchStatistics& stats) {
  m_archive_lock.Lock();
  m_archive.push_back(stats);
  m_archive_lock.Unlock();
}

std::vector<oaz::nn::EvaluationBatchStatistics>
oaz::nn::NNEvaluator::GetStatistics() {
  m_archive_lock.Lock();
  std::vector<oaz::nn::EvaluationBatchStatistics> archive(m_archive);
  m_archive_lock.Unlock();
  return archive;
}

void oaz::nn::NNEvaluator::ForceEvaluation() {
  m_batches.Lock();
  if (!m_batches.empty()) {
    auto earliest_batch = m_batches.front();
    earliest_batch->Lock();
    if (earliest_batch->IsAvailableForEvaluation()) {
      m_batches.pop_front();
      earliest_batch->Unlock();
      m_batches.Unlock();
      earliest_batch->GetStatistics().evaluation_forced = true;
      EvaluateBatch(earliest_batch.get());

    } else {
      earliest_batch->Unlock();
      m_batches.Unlock();
    }
  } else {
    m_batches.Unlock();
  }
}
