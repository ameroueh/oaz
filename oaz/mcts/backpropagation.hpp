#ifndef __BACKPROPAGATION_HPP__
#define __BACKPROPAGATION_HPP__

// I think this is dead code now?
namespace oaz::mcts {
float NormaliseScore(float score) { return (score + 1.) / 2.; }

float GetValueFromScore(float score, int player) {
  return (player == 0) ? score : 1. - score;
}
}  // namespace oaz::mcts
#endif
