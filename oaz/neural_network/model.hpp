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
			Model(): m_session(nullptr), m_run_semaphore(1) {
				initialise();	
			}

			void Load(std::string model_path) {
				MetaGraphDef graph_def;
				ReadBinaryProto(Env::Default(), model_path + "/graph.pb", &graph_def);
				m_session->Create(graph_def.graph_def());
				
				Tensor checkpoint_path_tensor(DT_STRING, TensorShape());
				checkpoint_path_tensor.scalar<std::string>()() = model_path + "/model";
				TF_CHECK_OK(
					m_session->Run(
						{{graph_def.saver_def().filename_tensor_name(), checkpoint_path_tensor},},
						{},
						{graph_def.saver_def().restore_op_name()},
						nullptr
					)
				);

			}
			
			void Checkpoint(std::string model_path) {
				Tensor checkpoint_path_tensor(DT_STRING, TensorShape());
				checkpoint_path_tensor.scalar<std::string>()() = model_path + "/model";
				
				TF_CHECK_OK(
					m_session->Run(
						{{"save/Const", checkpoint_path_tensor}},
						{},
						{"save/control_dependency"},
						nullptr
					)
				);
			}

			void Run(
				const std::vector<std::pair<std::string, Tensor> >& inputs,
				const std::vector<string>& output_tensor_names,
				const std::vector<string>& target_node_names,
				std::vector<Tensor>* outputs
			) {
				
				m_run_semaphore.lock();
				TF_CHECK_OK(
					m_session->Run(
						inputs,
						output_tensor_names,
						target_node_names,
						outputs
					)
				);
				m_run_semaphore.unlock();
			}
		private:
			using UniqueSessionPointer = std::unique_ptr<tensorflow::Session>;
			void initialise() {
				tensorflow::SessionOptions options;
				tensorflow::Session* session;
				TF_CHECK_OK(tensorflow::NewSession(options, &session));
				m_session.reset(session);
			}

			oaz::semaphore::SpinlockSemaphore m_run_semaphore;
			UniqueSessionPointer m_session;
	};
}
#endif
