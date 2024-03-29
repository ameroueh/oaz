#ifndef GAME_CLASS_NAME
  #error "GAME_CASS_NAME not defined!"
#endif
#ifndef GAME_HEADER
  #error "GAME_HEADER not defined!"
#endif
#ifndef MODULE_NAME
  #error "MODULE_NAME not defined!"
#endif

#define XSTRINGIFY(x) STRINGIFY(x)  // NOLINT
#define STRINGIFY(x) #x             // NOLINT

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>

#include "Python.h"

#include XSTRINGIFY(GAME_HEADER)

namespace p = boost::python;
namespace np = boost::python::numpy;

using GameImpl = oaz::games::GAME_CLASS_NAME;

static GameImpl CreateGameFromNDArray(const np::ndarray& array) {
  GameImpl game;
  game.InitialiseFromState(
      reinterpret_cast<float*>(array.get_data()));  // NOLINT
  return game;
}

static GameImpl CreateGameFromNDArrayCanonical(const np::ndarray& array) {
  GameImpl game;
  game.InitialiseFromCanonicalState(
      reinterpret_cast<float*>(array.get_data()));  // NOLINT
  return game;
}

p::list GetAvailableMoves(const GameImpl& game) {
  p::list l;
  std::vector<size_t> available_moves;
  game.GetAvailableMoves(&available_moves);
  for (auto move : available_moves) {
    l.append(move);
  }
  return l;
}

np::ndarray GetBoard(const GameImpl& game) {
  np::ndarray board = np::zeros(p::tuple(game.ClassMethods().GetBoardShape()),
                                np::dtype::get_builtin<float>());
  game.WriteStateToTensorMemory(
      reinterpret_cast<float*>(board.get_data()));  // NOLINT
  return board;
}

np::ndarray GetCanonicalBoard(const GameImpl& game) {
  np::ndarray board = np::zeros(p::tuple(game.ClassMethods().GetBoardShape()),
                                np::dtype::get_builtin<float>());
  game.WriteCanonicalStateToTensorMemory(
      reinterpret_cast<float*>(board.get_data()));  // NOLINT
  return board;
}

BOOST_PYTHON_MODULE(MODULE_NAME) {  // NOLINT
  PyEval_InitThreads();
  np::initialize();

  p::class_<GameImpl, p::bases<oaz::games::Game> >(XSTRINGIFY(GAME_CLASS_NAME))
      .def("play_move", &GameImpl::PlayMove)
      .def("from_numpy", &CreateGameFromNDArray)
      .staticmethod("from_numpy")
      .def("from_numpy_canonical", &CreateGameFromNDArrayCanonical)
      .staticmethod("from_numpy_canonical")
      .add_property("current_player", &GameImpl::GetCurrentPlayer)
      .add_property("finished", &GameImpl::IsFinished)
      .add_property("score", &GameImpl::GetScore)
      .add_property("available_moves", &GetAvailableMoves)
      .add_property("board", &GetBoard)
      .add_property("canonical_board", &GetCanonicalBoard);
}
