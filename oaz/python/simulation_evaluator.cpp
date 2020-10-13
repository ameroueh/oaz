#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/simulation/simulation_evaluator.hpp"

namespace p = boost::python;


BOOST_PYTHON_MODULE( simulation_evaluator ) {

	PyEval_InitThreads();

	p::class_<
		SimulationEvaluator,
		std::shared_ptr<SimulationEvaluator>, 
		boost::noncopyable
	>(
		"SimulationEvaluator",
		p::init<std::shared_ptr<oaz::thread_pool::ThreadPool>>()
	);
}
