#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>

#include <string>

#include "Python.h"

#include "oaz/games/bomberland/bomberland.hpp" 
#include "oaz/games/bomberland/json_state_builder.hpp"

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace oaz::games::bomberland;

p::list GetAvailableMoves(const Bomberland& game) {
  p::list l;
  std::vector<size_t> available_moves;
  game.GetAvailableMoves(&available_moves);
  for (auto move : available_moves) {
    l.append(move);
  }
  return l;
}

p::str GetStateAsJsonString(Bomberland& game) {
  const std::string output = JsonStateBuilder()(game).dump();
  return p::str(output.c_str(), output.size());
}

BOOST_PYTHON_MODULE(bomberland) {  // NOLINT
  PyEval_InitThreads();
  np::initialize();

  p::class_<Bomberland, p::bases<oaz::games::Game> >("Bomberland")
      .def("play_move", &Bomberland::PlayMove)
      .add_property("current_player", &Bomberland::GetCurrentPlayer)
      .add_property("finished", &Bomberland::IsFinished)
      .add_property("score", &Bomberland::GetScore)
      .add_property("available_moves", &GetAvailableMoves)
      .add_property("state_as_json_str", &GetStateAsJsonString);
}
