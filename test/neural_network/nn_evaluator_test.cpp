#include <fstream>
#include <iostream>

#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/neural_network/nn_testing.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "nlohmann/json.hpp"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include <string>

using namespace tensorflow;
using namespace oaz::nn;
using json = nlohmann::json;

TEST (Instantiation, Default) {
	NNEvaluator evaluator(64, 8, 8, 2, 1);
}

TEST (LoadGraph, Default) {
	NNEvaluator evaluator(1, 8, 8, 1, 1);
	evaluator.initialise("model");
}


TEST (Inference, CheckResults) {
	NNEvaluator evaluator(1, 2, 2, 1, 1);
	evaluator.initialise("model");

	std::ifstream ifs("data.json");
	json data = json::parse(ifs);
	
	auto batch_element = evaluator.getBatchElement(0);

	
	batch_element.loadFromJson(data[0]["input"]);
	
	evaluator.evaluate();

	ASSERT_EQ(batch_element.getValue(), data[0]["value"]);
	ASSERT_EQ(batch_element.getPolicy(), data[0]["policy"]);
}

TEST (Inference, Loop) {
	NNEvaluator evaluator(1, 2, 2, 1, 1);
	evaluator.initialise("model");
	
	std::ifstream ifs("data.json");
	json data = json::parse(ifs);
	
	auto batch_element = evaluator.getBatchElement(0);
	
	batch_element.loadFromJson(data[0]["input"]);

	for(int i=0; i!=100; ++i) {
		evaluator.evaluate();
	}
}
