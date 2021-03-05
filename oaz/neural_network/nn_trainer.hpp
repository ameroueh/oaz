#ifndef __NN_TRAINER_H__
#define __NN_TRAINER_H__

#include <iostream>

#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/training_batch.hpp"
#include "tensorflow/core/framework/tensor.h"

namespace oaz::nn {

template <class Game>
class NNTrainer {
 public:
  using SharedModelPointer = std::shared_ptr<Model>;

  NNTrainer(SharedModelPointer, size_t, size_t);
  void addTrainingExample(typename Game::Board*, typename Game::Value*,
                          typename Game::Policy*);

  std::string getStatus() const;

 private:
  using Batch = TrainingBatch<Game>;
  using UniqueBatchPointer = std::unique_ptr<Batch>;

  oaz::queue::SafeDeque<UniqueBatchPointer> m_batches;

  void trainFromBatch(Batch*);
  void maybeTrain();
  void addNewBatch();
  size_t getEpochSize() const;

  SharedModelPointer m_model;
  size_t m_batch_size;
  size_t m_epoch_size;

  float m_last_training_loss;
};
}  // namespace oaz::nn

#endif
