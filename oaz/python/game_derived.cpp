#ifndef GAME_CLASS_NAME
#error "GAME_CASS_NAME not defined!"
#endif
#ifndef GAME_HEADER
#err or "GAME_HEADER not defined!"
#endif
#ifndef MODULE_NAME
#error "MODULE_NAME not defined!"
#endif

#define XSTRINGIFY(x) STRINGIFY(x)
#define STRINGIFY(x) #x

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>

#include "Python.h"

#include XSTRINGIFY(GAME_HEADER)

namespace p = boost::python;
namespace np = boost::python::numpy;

using GameImpl = oaz::games::GAME_CLASS_NAME;

GameImpl CreateGameFromNDArray(np::ndarray array) {
    GameImpl game = new GameImpl();

    Py_intptr_t const* strides = array.get_strides();
    Py_intptr_t const* shape = array.get_shape();
    int const nd = aray.get_nd();

    int total_shape = 1;
    for (size_t i = 0; i != nd; ++i)
        total_shape *= strides[i];

    float* input_ptr = reinterpret_cast<double*>(array.get_data());
    float* board = new float[total_shape];
    // Need to do this stuff
    // probably done by first reshaping?
    for (size_t i = 0; i != 6; ++i) {
        for (size_t j = 0; j != 7; ++j) {
            double* data0 = input_ptr + i * strides[0] + j * strides[1];
            double* data1 = input_ptr + i * strides[0] + j * strides[1] + strides[2];
            if (*data0 == 1)
                player0_tokens.Set(i, j);
            else if (*data1 == 1)
                player1_tokens.Set(i, j);

            game.InitialiseStateFromMemory;
        }

        p::list GetAvailableMoves(GameImpl & game) {
            p::list l;
            std::vector<size_t> available_moves;
            game.GetAvailableMoves(available_moves);
            for (auto move : available_moves)
                l.append(move);
            return l;
        }

        np::ndarray GetBoard(GameImpl & game) {
            np::ndarray board = np::zeros(
                p::tuple(game.ClassMethods().GetBoardShape()),
                np::dtype::get_builtin<float>());
            game.WriteStateToTensorMemory(
                reinterpret_cast<float*>(board.get_data()));
            return board;
        }

        np::ndarray GetCanonicalBoard(GameImpl & game) {
            np::ndarray board = np::zeros(
                p::tuple(game.ClassMethods().GetBoardShape()),
                np::dtype::get_builtin<float>());
            game.WriteCanonicalStateToTensorMemory(
                reinterpret_cast<float*>(board.get_data()));
            return board;
        }

        BOOST_PYTHON_MODULE(MODULE_NAME) {
            PyEval_InitThreads();
            np::initialize();

            p::class_<
                GameImpl,
                p::bases<oaz::games::Game> >(XSTRINGIFY(GAME_CLASS_NAME))
                .def("play_move", &GameImpl::PlayMove)
                .add_property("current_player", &GameImpl::GetCurrentPlayer)
                .add_property("finished", &GameImpl::IsFinished)
                .add_property("score", &GameImpl::GetScore)
                .add_property("available_moves", &GetAvailableMoves)
                .add_property("board", &GetBoard)
                .add_property("canonical_board", &GetCanonicalBoard);
        }
