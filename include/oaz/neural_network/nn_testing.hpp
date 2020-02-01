#ifndef __NN_TESTING_HPP__
#define __NN_TESTING_HPP__

#include "nlohmann/json.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"

namespace testing::internal {

	bool operator==(const oaz::nn::PolicyType&, const nlohmann::json&);

}
#endif
