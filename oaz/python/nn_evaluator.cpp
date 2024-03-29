#include "oaz/neural_network/nn_evaluator.hpp"

#include "pybind11/pybind11.h"
/* #include "runtime.swg" */
/* #include "swigrun.swg" */
/* #include "python/pyhead.swg" */
/* #include "python/pyrun.swg" */
/* #include "python/pythreads.swg" */
/* #include "swigerrors.swg" */
/* #include "swiglabels.swg" */

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/stl_iterator.hpp>

#include "Python.h"
#include "oaz/neural_network/model.hpp"
#include "tensorflow/c/c_api.h"
#include "tensorflow/c/c_api_internal.h"

namespace p = boost::python;
namespace np = boost::python::numpy;
namespace py = pybind11;

namespace oaz::nn {

np::ndarray GetStatistics(oaz::nn::NNEvaluator* evaluator) {
  std::vector<oaz::nn::EvaluationBatchStatistics> stats =
      evaluator->GetStatistics();
  np::ndarray array = np::zeros(p::make_tuple(stats.size(), 6),  // NOLINT
                                np::dtype::get_builtin<size_t>());
  for (size_t i = 0; i != stats.size(); ++i) {
    array[i][0] = stats[i].time_created;
    array[i][1] = stats[i].time_evaluation_start;
    array[i][2] = stats[i].time_evaluation_end;
    array[i][3] = stats[i].n_elements;
    array[i][4] = stats[i].size;
    array[i][5] = stats[i].evaluation_forced ? 1 : 0;  // NOLINT
  }
  return array;
}
}  // namespace oaz::nn

/* inline void SetSessionV1(oaz::nn::Model& model, PyObject* obj) { */
/*   void* ptr = nullptr; */
/*   int result = SWIG_ConvertPtr(obj, &ptr, 0, 0); */
/*   TF_Session* session = static_cast<TF_Session*>(ptr); */
/*   model.SetSession(session->session); */
/* } */

void SetSessionV2(oaz::nn::Model* model, PyObject* obj) {
  py::handle handle(obj);
  TF_Session* session = handle.cast<TF_Session*>();
  model->SetSession(session->session);
}

std::shared_ptr<oaz::nn::NNEvaluator> ConstructNNEvaluator(
    const std::shared_ptr<oaz::nn::Model>& model, const p::object& cache,
    const std::shared_ptr<oaz::thread_pool::ThreadPool>& thread_pool,
    const p::object& dimensions, size_t batch_size) {
  p::stl_input_iterator<int> begin(dimensions);
  p::stl_input_iterator<int> end;
  std::vector<int> dimensions_vec(begin, end);

  std::shared_ptr<oaz::cache::Cache> cache_cxx(nullptr);
  if (!cache.is_none()) {
    cache_cxx = p::extract<std::shared_ptr<oaz::cache::Cache>>(cache);
  }
  return std::shared_ptr<oaz::nn::NNEvaluator>(new oaz::nn::NNEvaluator(
      model, cache_cxx, thread_pool, dimensions_vec, batch_size));
}

BOOST_PYTHON_MODULE(nn_evaluator) {  // NOLINT
  PyEval_InitThreads();

  /* auto pywrap_tf_session =
   * py::module::import("tensorflow.python._pywrap_tf_session"); */

  p::class_<oaz::nn::Model, std::shared_ptr<oaz::nn::Model>,
            boost::noncopyable>("Model", p::init<>())
      .def("set_session", &SetSessionV2)
      .add_property("input_node_name", &oaz::nn::Model::GetInputNodeName)
      .add_property("value_node_name", &oaz::nn::Model::GetValueNodeName)
      .add_property("policy_node_name", &oaz::nn::Model::GetPolicyNodeName)
      .def("set_input_node_name", &oaz::nn::Model::SetInputNodeName)
      .def("set_value_node_name", &oaz::nn::Model::SetValueNodeName)
      .def("set_policy_node_name", &oaz::nn::Model::SetPolicyNodeName);

  p::class_<oaz::nn::NNEvaluator, p::bases<oaz::evaluator::Evaluator>,
            std::shared_ptr<oaz::nn::NNEvaluator>, boost::noncopyable>(
      "NNEvaluator", p::no_init)
      .def("__init__", p::make_constructor(&ConstructNNEvaluator))
      .add_property("statistics", &oaz::nn::GetStatistics);
}
