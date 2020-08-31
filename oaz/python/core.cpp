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

#include "swiglabels.swg"
#include "swigrun.swg"
#include "swigerrors.swg"
#include "python/pythreads.swg"
#include "python/pyhead.swg"
#include "python/pyrun.swg"
#include "runtime.swg"

#include "tensorflow/c/c_api_internal.h"

#include XSTRINGIFY(GAME_HEADER)
#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/python/array_utils.hpp"

#include <iostream>

namespace p = boost::python;
namespace np = boost::python::numpy;

using Game = GAME_CLASS_NAME;
using Model = oaz::nn::Model;
using GameEvaluator = oaz::nn::NNEvaluator<Game>;
using GameSearch = oaz::mcts::AZSearch<Game>;
using Pool = oaz::thread_pool::ThreadPool;
using Node_ = oaz::mcts::SearchNode<Game::Move>;

void perform_search(GameSearch& search) {
	PyThreadState* save_state = PyEval_SaveThread();
	search.search();
	PyEval_RestoreThread(save_state);
}

p::list available_moves(Game& game) {

	p::list l;
	auto* available_moves = game.availableMoves();
	for(auto move : *available_moves)
		l.append(move);
	return l;
}

np::ndarray get_board(Game& game) {
	return oaz::python::ToNumpy(game.getBoard());
}

void set_session(Model& model, PyObject* obj) {
	void* ptr = nullptr;
	int result = SWIG_ConvertPtr(obj, &ptr, 0, 0);
	TF_Session* session = static_cast<TF_Session*>(ptr);
	model.setSession(session->session);
}


BOOST_PYTHON_MODULE( MODULE_NAME ) {

	PyEval_InitThreads();
	np::initialize();

	p::class_<Game>( XSTRINGIFY(GAME_CLASS_NAME) )
		.def("play_move", &Game::playMove)
		.def("undo_move", &Game::undoMove)
		.add_property("current_player", &Game::getCurrentPlayer)
		.add_property("finished", &Game::Finished)
		.add_property("score", &Game::score)
		.add_property("available_moves", available_moves)
		.add_property("board", &get_board);
	
	p::class_<Model, std::shared_ptr<Model>, boost::noncopyable>("Model", p::init<>() )
		.def("set_session", &set_session)
		.add_property("value_node_name", &Model::getValueNodeName)
		.add_property("policy_node_name", &Model::getPolicyNodeName)
		.def("set_value_node_name", &Model::setValueNodeName)
		.def("set_policy_node_name", &Model::setPolicyNodeName);
	
	p::class_<GameEvaluator, std::shared_ptr<GameEvaluator>, boost::noncopyable>(
		"Evaluator", 
		p::init<std::shared_ptr<Model>, 
		oaz::thread_pool::ThreadPool*,
		size_t>());

	p::class_<Node_, boost::noncopyable>("Node", p::init<>())
		.add_property("move", &Node_::getMove)
		.add_property("is_root", &Node_::isRoot)
		.add_property("is_leaf", &Node_::isLeaf)
		.add_property("n_children", &Node_::getNChildren)
		.add_property("n_visits", &Node_::getNVisits)
		.add_property("accumulated_value", &Node_::getAccumulatedValue)
		.def("get_child", &Node_::getChild, p::return_value_policy<p::reference_existing_object>())
		.def("get_parent", &Node_::getParent, p::return_value_policy<p::reference_existing_object>());


	p::class_<GameSearch, std::shared_ptr<GameSearch>, boost::noncopyable>(
		"Search", 
		p::init<const Game&, GameEvaluator*, oaz::thread_pool::ThreadPool*, size_t, size_t, float, float>())
	.def("get_root", &GameSearch::getTreeRoot, p::return_value_policy<p::reference_existing_object>())
	.add_property("done", &GameSearch::done)
	.def("search", &perform_search)
	.def("seed_rng", &GameSearch::seedRNG);
	
	p::class_<Pool, shared_ptr<Pool>, boost::noncopyable>("Pool", p::init<size_t>());

}
