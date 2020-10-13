#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/thread_pool/thread_pool.hpp"

namespace p = boost::python;

BOOST_PYTHON_MODULE( thread_pool ) {

	PyEval_InitThreads();

	p::class_<
		oaz::thread_pool::ThreadPool,
		std::shared_ptr<oaz::thread_pool::ThreadPool>,
		boost::noncopyable
	>("ThreadPool", p::init<size_t>());
}
