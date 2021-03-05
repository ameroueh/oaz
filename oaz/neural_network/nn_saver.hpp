#ifndef __NN_SAVER_H__
#define __NN_SAVER_H__

#include "oaz/neural_network/training_batch.hpp"
#include "tensorflow/core/framework/tensor.h"

namespace oaz::nn {

template <class Game>
class NNSaver {
 public:
  NNTrainer(size_t, size_t);
  void addTrainingExample(typename Game::Board*, typename Game::Value*,
                          typename Game::Policy*);

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
};
}  // namespace oaz::nn

#include "nn_trainer_impl.cpp"
#endif
