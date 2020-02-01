#include "oaz/neural_network/nn_testing.hpp"

bool testing::internal::operator==(const oaz::nn::PolicyType& policy, const nlohmann::json& expected_policy) {

	for(uint32_t i=0; i!= policy.dimensions()[0]; ++i)
		if(policy(i) != expected_policy[i])
			return false;
	return true;
}
