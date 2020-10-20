#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/cache/cache.hpp"

namespace p = boost::python;


BOOST_PYTHON_MODULE( cache ) {

	PyEval_InitThreads();

	p::class_<
		oaz::cache::Cache,
		boost::noncopyable
	>(
		"Cache",
		p::no_init
	);
}
