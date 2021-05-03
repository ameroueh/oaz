#include "oaz/thread_pool/thread_pool.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

BOOST_PYTHON_MODULE(thread_pool) {  // NOLINT
  PyEval_InitThreads();

  p::class_<oaz::thread_pool::ThreadPool,
            std::shared_ptr<oaz::thread_pool::ThreadPool>, boost::noncopyable>(
      "ThreadPool", p::init<size_t>());
}
