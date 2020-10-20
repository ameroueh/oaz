#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/stl_iterator.hpp>

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


std::shared_ptr<oaz::nn::NNEvaluator> ConstructNNEvaluator(
	std::shared_ptr<oaz::nn::Model> model,
	p::object cache,
	std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
	const p::object& dimensions,
	size_t batch_size) {
		
		p::stl_input_iterator<int> begin(dimensions), end;
		std::vector<int> dimensions_vec(begin, end);

		std::shared_ptr<oaz::cache::Cache> cache_cxx(nullptr);
		if(!cache.is_none())
			cache_cxx = p::extract<std::shared_ptr<oaz::cache::Cache>>(cache);
		return std::shared_ptr<oaz::nn::NNEvaluator>(
			new oaz::nn::NNEvaluator(
				model,
				cache_cxx,
				thread_pool,
				dimensions_vec,
				batch_size
			)
		);
}

BOOST_PYTHON_MODULE( nn_evaluator ) {

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
		p::bases<oaz::evaluator::Evaluator>,
		std::shared_ptr<oaz::nn::NNEvaluator>,
		boost::noncopyable
	>(
		"NNEvaluator", 
		p::no_init
	).def(
		"__init__",
		p::make_constructor(&ConstructNNEvaluator)
	);
}
