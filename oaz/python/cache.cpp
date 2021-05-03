#include "oaz/cache/cache.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

BOOST_PYTHON_MODULE(cache) { // NOLINT
  PyEval_InitThreads();

  p::class_<oaz::cache::Cache, boost::noncopyable>("Cache", p::no_init);
}
