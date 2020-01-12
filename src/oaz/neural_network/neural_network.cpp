#include "oaz/neural_network/neural_network.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/platform/env.h"


using namespace std;
using namespace tensorflow;

void oaz::nn::loadModel(ClientSession* session, string graph_path, string checkpoint_path) {
	
	GraphDef graph_def;
	ReadBinaryProto(Env::Default(), graph_path, &graph_def);
}

void oaz::nn::loadModel(ClientSession* session, string graph_path) {
	
	GraphDef graph_def;
	ReadBinaryProto(Env::Default(), graph_path, &graph_def);
}
