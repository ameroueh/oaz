#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/evaluator/evaluator.hpp"

namespace p = boost::python;


BOOST_PYTHON_MODULE( evaluator ) {

	PyEval_InitThreads();

	p::class_<
		oaz::evaluator::Evaluator,
		boost::noncopyable
	>(
		"Evaluator",
		p::no_init
	);
}
