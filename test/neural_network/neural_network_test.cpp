#include "tensorflow/cc/client/client_session.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

#include "oaz/neural_network/neural_network.h"

using namespace tensorflow;
using namespace oaz::nn;

TEST (LoadModel, Dummy) {

	Scope root = Scope::NewRootScope();
	ClientSession session(root);
	loadModel(&session, "graph.pb", "");

}

TEST (LoadModel, GraphOnly) {

	Scope root = Scope::NewRootScope();
	ClientSession session(root);
	loadModel(&session, "graph.pb");
}
