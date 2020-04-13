#ifndef __BACKPROPAGATION_HPP__
#define __BACKPROPAGATION_HPP__

namespace oaz::mcts {
	float normaliseScore(float score) {
		return (score + 1.) / 2.;
	}

	float getValueFromScore(float score, int current_player) {
		return (current_player == 1) ? score : 1. - score;
		
	}
}
#endif
