#include "oaz/games/game.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <vector>

#include "Python.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

BOOST_PYTHON_MODULE(game) {
    PyEval_InitThreads();

    p::class_<std::vector<int> >("IntVec")
        .def(p::vector_indexing_suite<std::vector<int> >());

    p::class_<
        oaz::games::Game,
        boost::noncopyable>("Game", p::init<>());
    // .def(p::init<np::ndarray>());
}

// #include "oaz/games/game.hpp"

// #include <boost/python.hpp>
// #include <boost/python/def.hpp>
// #include <boost/python/module.hpp>
// #include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// #include "Python.h"

// namespace p = boost::python;
// // namespace np = boost::python::numpy;

// BOOST_PYTHON_MODULE(game) {
//     PyEval_InitThreads();

//     p::class_<std::vector<int> >("IntVec")
//         .def(p::vector_indexing_suite<std::vector<int> >());

//     p::class_<
//         oaz::games::Game,
//         boost::noncopyable>("Game", p::no_init);
//     // .def(init<np::ndarray input_board>);
// }
