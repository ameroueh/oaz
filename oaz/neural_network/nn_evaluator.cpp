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
      m_evaluations(boost::extents[size]),
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
    size_t index, oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
    oaz::thread_pool::Task* task) {
  float* destination = m_batch.flat<float>().data() + index * GetElementSize();
  game->WriteCanonicalStateToTensorMemory(destination);
  m_games[index] = game;
  m_evaluations[index] = evaluation;
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

boost::multi_array_ref<std::unique_ptr<oaz::evaluator::Evaluation>*, 1>
oaz::nn::EvaluationBatch::GetEvaluations() {
  return boost::multi_array_ref<
      std::unique_ptr<oaz::evaluator::Evaluation>*, 1>(
      m_evaluations.origin(), boost::extents[GetSize()]);
}

std::unique_ptr<oaz::evaluator::Evaluation>* oaz::nn::EvaluationBatch::GetEvaluation(size_t index) {
  return m_evaluations[index];
}

size_t oaz::nn::EvaluationBatch::GetSize() const { return m_size; }

size_t oaz::nn::EvaluationBatch::GetElementSize() const {
  return m_element_size;
}

tensorflow::Tensor& oaz::nn::EvaluationBatch::GetBatchTensor() {
  return m_batch;
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
    oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation, oaz::thread_pool::Task* task) {
  if (m_cache && EvaluateFromCache(game, evaluation, task)) {
    return;
  }
  EvaluateFromNN(game, evaluation, task);
}

bool oaz::nn::NNEvaluator::EvaluateFromCache(
    oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
    oaz::thread_pool::Task* task) {
  bool success = m_cache->Evaluate(*game, evaluation);
  if (success) {
    m_thread_pool->enqueue(task);
  }
  return success;
}

void oaz::nn::NNEvaluator::EvaluateFromNN(
    oaz::games::Game* game,
    std::unique_ptr<oaz::evaluator::Evaluation>* evaluation,
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

    current_batch->InitialiseElement(index, game, evaluation, task);

    if (evaluate_batch) {
      while (!current_batch->IsAvailableForEvaluation()) {
      }
      EvaluateBatch(current_batch.get());
    }

  } else {
    AddNewBatch();
    m_batches.Unlock();

    EvaluateFromNN(game, evaluation, task);
  }
}

void oaz::nn::NNEvaluator::EvaluateBatch(oaz::nn::EvaluationBatch* batch) {
  batch->GetStatistics().time_evaluation_start = oaz::utils::time_now_ns();
  batch->GetStatistics().n_elements = batch->GetNumberOfElements();

  auto outputs = std::make_shared<std::vector<tensorflow::Tensor>>();

  m_n_evaluation_requests++;
  m_model->Run(
      {{m_model->GetInputNodeName(),
        batch->GetBatchTensor().Slice(0, batch->GetNumberOfElements())}},
      {m_model->GetValueNodeName(), m_model->GetPolicyNodeName()}, {},
      outputs.get());
  m_n_evaluations++;

  for (size_t i = 0; i != batch->GetNumberOfElements(); ++i) {
    *(batch->GetEvaluation(i)) = std::move(
       std::make_unique<DefaultNNEvaluation>(outputs, i) 
    );
  }

  if (m_cache) {
    m_cache->BatchInsert(batch->GetGames(),
                         batch->GetEvaluations(), batch->GetNumberOfElements());
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

oaz::nn::DefaultNNEvaluation::DefaultNNEvaluation(std::shared_ptr<std::vector<tensorflow::Tensor>> outputs, size_t index): m_outputs(outputs), m_index(index) {}

float oaz::nn::DefaultNNEvaluation::GetValue() const {
  return (*(m_outputs.get()))[0].template tensor<float, 2>()(m_index, 0);
}

float oaz::nn::DefaultNNEvaluation::GetPolicy(size_t move) const {
  return (*(m_outputs.get()))[1].template tensor<float, 2>()(m_index, move);
}

std::unique_ptr<oaz::evaluator::Evaluation> oaz::nn::DefaultNNEvaluation::Clone() const {
  return std::make_unique<DefaultNNEvaluation>(*this);
}
