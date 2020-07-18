#ifndef __NN_MODEL_H__
#define __NN_MODEL_H__

#include <string>
#include <vector>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/saved_model/loader.h"

#include "oaz/semaphore/semaphore.hpp"

using namespace tensorflow;

namespace oaz::nn {

	class Model {
		public:
			Model(): m_session(nullptr) {}
		
			void setSession(tensorflow::Session* session) {
				m_session = session;
			}

			void setPolicyNodeName(std::string policy_node_name) {
				m_policy_node_name = policy_node_name;
			}
			
			void setValueNodeName(std::string value_node_name) {
				m_value_node_name = value_node_name;
			}

			std::string getPolicyNodeName() const {
				return m_policy_node_name;
			}
			
			std::string getValueNodeName() const {
				return m_value_node_name;
			}

			void Run(
				const std::vector<std::pair<std::string, Tensor> >& inputs,
				const std::vector<string>& output_tensor_names,
				const std::vector<string>& target_node_names,
				std::vector<Tensor>* outputs
			) {
				TF_CHECK_OK(
					m_session->Run(
						inputs,
						output_tensor_names,
						target_node_names,
						outputs
					)
				);
			}
		private:
			tensorflow::Session* m_session;
			std::string m_policy_node_name;
			std::string m_value_node_name;
	};

	tensorflow::Session* createSession() {
		tensorflow::SessionOptions options;
		tensorflow::Session* session;
		TF_CHECK_OK(tensorflow::NewSession(options, &session));
		return session;
	}

	void loadGraph(tensorflow::Session* session, std::string path) {
		GraphDef graph_def;
		ReadBinaryProto(Env::Default(), path, &graph_def);
		TF_CHECK_OK(session->Create(graph_def));
	}

	tensorflow::Session* createSessionAndLoadGraph(std::string path) {
		tensorflow::Session* session(createSession());
		loadGraph(session, path);
		return session;
	}

	Model* createModel(tensorflow::Session* session, std::string value_node_name, std::string policy_node_name) {
		Model* model(new Model());
		model->setSession(session);
		model->setValueNodeName("value");
		model->setPolicyNodeName("policy");
		return model;
	}
}
#endif
