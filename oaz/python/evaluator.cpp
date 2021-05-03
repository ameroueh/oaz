#include "oaz/evaluator/evaluator.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

BOOST_PYTHON_MODULE(evaluator) { // NOLINT
  PyEval_InitThreads();

  p::class_<oaz::evaluator::Evaluator, boost::noncopyable>("Evaluator",
                                                           p::no_init);
}
