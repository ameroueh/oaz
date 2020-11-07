#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/games/game.hpp"

namespace p = boost::python;

BOOST_PYTHON_MODULE( game ) {

	PyEval_InitThreads();
	p::class_<
		oaz::games::Game,
		boost::noncopyable
	>( "Game", p::no_init);

}