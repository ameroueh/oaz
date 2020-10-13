#ifndef GAME_CLASS_NAME
	#error "GAME_CASS_NAME not defined!"
#endif
#ifndef GAME_HEADER
	#error "GAME_HEADER not defined!"
#endif
#ifndef MODULE_NAME
	#error "MODULE_NAME not defined!"
#endif


#define XSTRINGIFY(x) STRINGIFY(x)
#define STRINGIFY(x) #x

#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/numpy.hpp>
#include XSTRINGIFY(GAME_HEADER)

namespace p = boost::python;
namespace np = boost::python::numpy;

using GameImpl = oaz::games::GAME_CLASS_NAME;

p::list GetAvailableMoves(GameImpl& game) {

	p::list l;
	std::vector<size_t> available_moves;
	game.GetAvailableMoves(available_moves);
	for(auto move : available_moves)
		l.append(move);
	return l;
}

np::ndarray GetBoard(GameImpl& game) {
	np::ndarray board = np::zeros(
		p::tuple(game.ClassMethods().GetBoardShape()),
		np::dtype::get_builtin<float>()
	);
	game.WriteStateToTensorMemory(
		reinterpret_cast<float*>(board.get_data())
	);
	return board;
}

BOOST_PYTHON_MODULE( MODULE_NAME ) {

	PyEval_InitThreads();
	np::initialize();

	p::class_<GameImpl>( XSTRINGIFY(GAME_CLASS_NAME) )
		.def("play_move", &GameImpl::PlayMove)
		.add_property("current_player", &GameImpl::GetCurrentPlayer)
		.add_property("finished", &GameImpl::IsFinished)
		.add_property("score", &GameImpl::GetScore)
		.add_property("available_moves", &GetAvailableMoves)
		.add_property("board", &GetBoard);
}
