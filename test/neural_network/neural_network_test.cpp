/* #include "tensorflow/cc/client/client_session.h" */
#include "tensorflow/core/public/session.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "nlohmann/json.hpp"

#include <fstream>
#include <string>
#include <utility>

#include "oaz/neural_network/neural_network.h"

using namespace tensorflow;
using namespace oaz::nn;
using json = nlohmann::json;

typedef std::vector<std::pair<std::string, tensorflow::Tensor>> TensorDict;

TEST (LoadModel, LoadGraphAndRun) {

	tensorflow::SessionOptions options;
	std::unique_ptr<tensorflow::Session> session(NewSession(options));
	
	GraphDef graph_def;
	ReadBinaryProto(Env::Default(), "graph.pb", &graph_def);

	session->Create(graph_def);
	
	Tensor inputData(DT_FLOAT, TensorShape({1, 2}));
	auto inputDataView = inputData.tensor<float, 2>();

	std::ifstream ifs("data.json");
	json data = json::parse(ifs);
	for(int i=0; i!=2; ++i)
		inputDataView(0, i) = data["input"][0][i];

	std::vector<Tensor> outputs;
	session->Run({{"input:0", inputData}}, {"output"}, {}, &outputs);

	auto mat = outputs[0].matrix<float>();
	ASSERT_EQ(data["output"][0][0], mat(0, 0));
	ASSERT_EQ(data["output"][0][1], mat(0, 1));
}
