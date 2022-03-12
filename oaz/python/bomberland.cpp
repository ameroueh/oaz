#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>

#include "Python.h"

#include "oaz/games/bomberland/bomberland.hpp" 
#include "oaz/games/bomberland/json_state_builder.hpp"

namespace p = boost::python;
namespace np = boost::python::numpy;

using Bomberland = oaz::games::Bomberland;

/* static Bomberland CreateGameFromNDArray(const np::ndarray& array) { */
/*   Bomberland game; */
/*   game.InitialiseFromState( */
/*       reinterpret_cast<float*>(array.get_data()));  // NOLINT */
/*   return game; */
/* } */

/* static Bomberland CreateGameFromNDArrayCanonical(const np::ndarray& array) { */
/*   Bomberland game; */
/*   game.InitialiseFromCanonicalState( */
/*       reinterpret_cast<float*>(array.get_data()));  // NOLINT */
/*   return game; */
/* } */

p::list GetAvailableMoves(const Bomberland& game) {
  p::list l;
  std::vector<size_t> available_moves;
  game.GetAvailableMoves(&available_moves);
  for (auto move : available_moves) {
    l.append(move);
  }
  return l;
}

p::str GetStateAsJsonString(const Bomberland& game) {
  return JsonStateBuilder()(game).dump();
}

/* np::ndarray GetBoard(const Bomberland& game) { */
/*   np::ndarray board = np::zeros(p::tuple(game.ClassMethods().GetBoardShape()), */
/*                                 np::dtype::get_builtin<float>()); */
/*   game.WriteStateToTensorMemory( */
/*       reinterpret_cast<float*>(board.get_data()));  // NOLINT */
/*   return board; */
/* } */

/* np::ndarray GetCanonicalBoard(const Bomberland& game) { */
/*   np::ndarray board = np::zeros(p::tuple(game.ClassMethods().GetBoardShape()), */
/*                                 np::dtype::get_builtin<float>()); */
/*   game.WriteCanonicalStateToTensorMemory( */
/*       reinterpret_cast<float*>(board.get_data()));  // NOLINT */
/*   return board; */
/* } */

BOOST_PYTHON_MODULE(MODULE_NAME) {  // NOLINT
  PyEval_InitThreads();
  np::initialize();

  p::class_<Bomberland, p::bases<oaz::games::Game> >("Bomberland")
      .def("play_move", &Bomberland::PlayMove)
      /* .def("from_numpy", &CreateGameFromNDArray) */
      /* .staticmethod("from_numpy") */
      /* .def("from_numpy_canonical", &CreateGameFromNDArrayCanonical) */
      /* .staticmethod("from_numpy_canonical") */
      .add_property("current_player", &Bomberland::GetCurrentPlayer)
      .add_property("finished", &Bomberland::IsFinished)
      .add_property("score", &Bomberland::GetScore)
      .add_property("available_moves", &GetAvailableMoves)
      .add_property("state_as_json_str", &GetStateAsJsonString)
      /* .add_property("board", &GetBoard) */
      /* .add_property("canonical_board", &GetCanonicalBoard); */
}
