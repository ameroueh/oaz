#include "oaz/mcts/selection.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

BOOST_PYTHON_MODULE(selection) {
  PyEval_InitThreads();

  p::class_<oaz::mcts::Selector, boost::noncopyable>("Selector", p::no_init);

  p::class_<oaz::mcts::UCTSelector, p::bases<oaz::mcts::Selector> >(
      "UCTSelector");
  p::class_<oaz::mcts::AZSelector, p::bases<oaz::mcts::Selector> >(
      "AZSelector");
}
