#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "swiglabels.swg"
#include "swigrun.swg"
#include "swigerrors.swg"
#include "python/pythreads.swg"
#include "python/pyhead.swg"
#include "python/pyrun.swg"
#include "runtime.swg"

#include "tensorflow/c/c_api_internal.h"

#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"

namespace p = boost::python;

void SetSession(Model& model, PyObject* obj) {
	void* ptr = nullptr;
	int result = SWIG_ConvertPtr(obj, &ptr, 0, 0);
	TF_Session* session = static_cast<TF_Session*>(ptr);
	model.SetSession(session->session);
}

BOOST_PYTHON_MODULE( pyoaz_connect_four_core ) {

	PyEval_InitThreads();

	p::class_<
		oaz::nn::Model,
		std::shared_ptr<oaz::nn::Model>,
		boost::noncopyable
	>("Model", p::init<>())
	.def("set_session", &SetSession)
	.add_property("value_node_name", &oaz::nn::Model::GetValueNodeName)
	.add_property("policy_node_name", &oaz::nn::Model::GetPolicyNodeName)
	.def("set_value_node_name", &oaz::nn::Model::SetValueNodeName)
	.def("set_policy_node_name", &oaz::nn::Model::SetPolicyNodeName);

	p::class_<
		oaz::nn::NNEvaluator, 
		std::shared_ptr<oaz::nn::NNEvaluator>,
		boost::noncopyable
	>(
		"NNEvaluator", 
		p::init<std::shared_ptr<oaz::nn::Model>, 
		std::shared_ptr<oaz::thread_pool::ThreadPool>,
		const std::vector<int>&,
		size_t>()
	);
}
