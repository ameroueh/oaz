#include <string>

#include "stdint.h"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/protobuf/meta_graph.pb.h"

#include "oaz/neural_network/nn_evaluator.hpp"

#include <iostream>

using namespace oaz::nn;
using namespace tensorflow;
using namespace std;
using json = nlohmann::json;

NNEvaluator::NNEvaluator(esize_t batch_size, esize_t batch_element_width, esize_t batch_element_length, esize_t batch_element_height, esize_t policy_size):
	m_batch_size(batch_size),
	m_batch_element_width(batch_element_width),
	m_batch_element_length(batch_element_length),
	m_batch_element_height(batch_element_height),
	m_policy_size(policy_size),
	m_batch_tensor(
		DT_FLOAT, 
		TensorShape(
			{batch_size, batch_element_width, batch_element_length, batch_element_height}
		)
	),
	m_batch(m_batch_tensor.tensor<float, 4>()),
	m_session(nullptr)
	{
	}


void NNEvaluator::initialise(string model_path) {
	TF_CHECK_OK(LoadSavedModel({}, {}, model_path, {"serve"}, &m_saved_model_bundle));
	m_session = m_saved_model_bundle.session.get();
}

esize_t NNEvaluator::getWidth() const {
	return m_batch_element_width;
}

esize_t NNEvaluator::getLength() const {
	return m_batch_element_length;
}

esize_t NNEvaluator::getHeight() const {
	return m_batch_element_height;
}

void NNEvaluator::evaluate() {
	TF_CHECK_OK(m_session->Run(
		{{"input:0", m_batch_tensor}},
		{"value", "policy"},
		{},
		&m_outputs
	));
}

ValueType NNEvaluator::getValue(esize_t index) const {
	return m_outputs[0].tensor<float, 1>()(index);
}

PolicyType NNEvaluator::getPolicy(esize_t index) {
	return PolicyType(&m_outputs[1].tensor<float, 2>()(index, 0), m_policy_size);
}

NNEvaluator::~NNEvaluator() {}

BatchElement NNEvaluator::getBatchElement(esize_t index) {
	return BatchElement(index, &m_batch, this);
}


BatchElement::BatchElement(esize_t index, BatchType* batch, NNEvaluator* evaluator): m_index(index), m_batch(batch), m_evaluator(evaluator) {}

float& BatchElement::operator()(esize_t i, esize_t j, esize_t k) {
	return (*m_batch)(m_index, i, j, k);
}

void BatchElement::loadFromJson(const json& data) {
	for(esize_t i=0; i!=m_evaluator->getWidth(); ++i)
		for(esize_t j=0; j!=m_evaluator->getLength(); ++j) 
			for(esize_t k=0; k!=m_evaluator->getHeight(); ++k) 
				operator()(i, j, k) = data[i][j][k];
}

ValueType BatchElement::getValue() const {
	return m_evaluator->getValue(m_index);
}

PolicyType BatchElement::getPolicy() {
	return m_evaluator->getPolicy(m_index);
}
