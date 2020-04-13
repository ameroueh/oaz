#ifndef __NN_TESTING_HPP__
#define __NN_TESTING_HPP__

#include "nlohmann/json.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"

namespace testing::internal {
	
	/* template<class Game> */
	/* bool operator==(const typename oaz::nn::NNEvaluator<Game>::PolicyType& policy, const nlohmann::json& expected_policy) { */
	/* for(size_t i=0; i!= policy.dimensions()[0]; ++i) */
	/* 	if(policy(i) != expected_policy[i]) */
	/* 		return false; */
	/* return true; */
	/* } */

}
#endif
