#ifndef OAZ_NEURAL_NETWORK_MODEL_HPP_
#define OAZ_NEURAL_NETWORK_MODEL_HPP_

#include <memory>
#include <string>
#include <vector>

#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

namespace oaz::nn {

class Model {
 public:
  Model() : m_session(nullptr) {}

  void SetSession(tensorflow::Session* session) { m_session = session; }

  void SetPolicyNodeName(std::string policy_node_name) {
    m_policy_node_name = policy_node_name;
  }

  void SetInputNodeName(std::string input_node_name) {
    m_input_node_name = input_node_name;
  }

  void SetValueNodeName(std::string value_node_name) {
    m_value_node_name = value_node_name;
  }

  std::string GetInputNodeName() const { return m_input_node_name; }

  std::string GetPolicyNodeName() const { return m_policy_node_name; }

  std::string GetValueNodeName() const { return m_value_node_name; }

  void Run(
      const std::vector<std::pair<std::string, tensorflow::Tensor> >& inputs,
      const std::vector<std::string>& output_tensor_names,
      const std::vector<std::string>& target_node_names,
      std::vector<tensorflow::Tensor>* outputs) {
    TF_CHECK_OK(m_session->Run(inputs, output_tensor_names, target_node_names,
                               outputs));
  }

 private:
  tensorflow::Session* m_session;
  std::string m_policy_node_name;
  std::string m_value_node_name;
  std::string m_input_node_name;
};

tensorflow::Session* CreateSession() {
  tensorflow::SessionOptions options;
  tensorflow::Session* session;
  TF_CHECK_OK(tensorflow::NewSession(options, &session));
  return session;
}

void LoadGraph(tensorflow::Session* session, std::string path) {
  tensorflow::GraphDef graph_def;
  ReadBinaryProto(tensorflow::Env::Default(), path, &graph_def);
  TF_CHECK_OK(session->Create(graph_def));
}

tensorflow::Session* CreateSessionAndLoadGraph(std::string path) {
  tensorflow::Session* session(CreateSession());
  LoadGraph(session, path);
  return session;
}

std::shared_ptr<Model> CreateModel(tensorflow::Session* session,
                                   std::string input_node_name,
                                   std::string value_node_name,
                                   std::string policy_node_name) {
  auto model = std::make_shared<Model>();
  model->SetSession(session);
  model->SetValueNodeName("value");
  model->SetPolicyNodeName("policy");
  model->SetInputNodeName("input");
  return model;
}
}  // namespace oaz::nn
#endif  // OAZ_NEURAL_NETWORK_MODEL_HPP_
