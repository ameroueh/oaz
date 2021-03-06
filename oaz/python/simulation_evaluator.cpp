#include "oaz/simulation/simulation_evaluator.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

BOOST_PYTHON_MODULE(simulation_evaluator) {
  PyEval_InitThreads();

  p::class_<oaz::simulation::SimulationEvaluator,
            p::bases<oaz::evaluator::Evaluator>,
            std::shared_ptr<oaz::simulation::SimulationEvaluator>,
            boost::noncopyable>(
      "SimulationEvaluator",
      p::init<std::shared_ptr<oaz::thread_pool::ThreadPool>>());
}
