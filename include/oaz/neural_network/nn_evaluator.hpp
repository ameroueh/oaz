#ifndef __NN_EVALUATOR_H__
#define __NN_EVALUATOR_H__

#include <string>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/saved_model/loader.h"

#include "nlohmann/json.hpp"


namespace oaz::nn {

	using esize_t = uint32_t;
	using PolicyType = tensorflow::TTypes<float, 1>::Tensor;
		/* TensorMap<Tensor<float, 1>>; */
	using ValueType = float;
	using BatchType = tensorflow::TTypes<float, 4>::Tensor;
	using BatchValueType = tensorflow::TTypes<float, 1>::Tensor;
	using BatchPolicyType = tensorflow::TTypes<float, 2>::Tensor;

	class NNEvaluator;

	class BatchElement {
		public:
			BatchElement(esize_t, BatchType*, NNEvaluator*);
			float& operator()(esize_t, esize_t, esize_t);
			void loadFromJson(const nlohmann::json&);
			ValueType getValue() const;
			PolicyType getPolicy();
		private:
			esize_t m_index;
			BatchType* m_batch;
			NNEvaluator* m_evaluator;
	};

	class NNEvaluator {
		public:
			NNEvaluator(esize_t, esize_t, esize_t, esize_t, esize_t);

			void initialise(std::string);
			BatchElement getBatchElement(esize_t);
			void evaluate();

			esize_t getWidth() const;
			esize_t getLength() const;
			esize_t getHeight() const;

			~NNEvaluator();

		private:
			tensorflow::Tensor m_batch_tensor;
			std::vector<tensorflow::Tensor> m_outputs;
			esize_t m_batch_size;
			esize_t m_batch_element_width;
			esize_t m_batch_element_length;
			esize_t m_batch_element_height;
			esize_t m_policy_size;
			BatchType m_batch;
			
			ValueType getValue(esize_t) const;
			PolicyType getPolicy(esize_t);

			tensorflow::SavedModelBundle m_saved_model_bundle;
			tensorflow::Session* m_session;

			friend class BatchElement;
	};

}

#endif // __NN_EVALUATOR_H__
