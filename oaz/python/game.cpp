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

#include "oaz/games/connect_four.hpp"
#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"

#include <iostream>

namespace p = boost::python;
namespace np = boost::python::numpy;

using Game = ConnectFour;
using Model = oaz::nn::Model;
using Evaluator = oaz::nn::NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using Search_ = oaz::mcts::AZSearch<Game, Evaluator>;
using SearchPool = oaz::mcts::AZSearchPool<Game, Evaluator>;
using Node_ = oaz::mcts::SearchNode<Game::Move>;

void perform_search(SearchPool& pool, Search_* search) {
	PyThreadState* save_state = PyEval_SaveThread();
	pool.performSearch(search);
	PyEval_RestoreThread(save_state);
}

np::ndarray get_board(Game& game) {
	return np::from_data(
		&(game.getBoard()[0][0][0]),
		np::dtype::get_builtin<float>(),
		p::make_tuple(7, 6, 2),
		p::make_tuple(6*2*sizeof(float), 2*sizeof(float), sizeof(float)),
		p::object()
	);
}

void set_session(Model& model, PyObject* obj) {
	void* ptr = nullptr;
	int result = SWIG_ConvertPtr(obj, &ptr, 0, 0);
	TF_Session* session = static_cast<TF_Session*>(ptr);
	model.setSession(session->session);
}


BOOST_PYTHON_MODULE(az_connect_four) {

	PyEval_InitThreads();
	np::initialize();

	p::class_<Game>("ConnectFour")
		.def("play_move", &Game::playMove)
		.def("undo_move", &Game::undoMove)
		.def("current_player", &Game::getCurrentPlayer)
		.def("finished", &Game::Finished)
		.def("score", &Game::score)
		.def("get_policy_size", &Game::getPolicySize)
		.def("get_board", &get_board);
	
	p::class_<Model, std::shared_ptr<Model>, boost::noncopyable>("Model", p::init<>() )
		.def("set_session", &set_session)
		.def("get_value_node_name", &Model::getValueNodeName)
		.def("get_policy_node_name", &Model::getPolicyNodeName)
		.def("set_value_node_name", &Model::setValueNodeName)
		.def("set_policy_node_name", &Model::setPolicyNodeName);
	
	p::class_<Evaluator, std::shared_ptr<Evaluator>, boost::noncopyable>("Evaluator", p::init<std::shared_ptr<Model>, size_t>());

	p::class_<Node_, boost::noncopyable>("Node", p::init<>())
		.def("get_move", &Node_::getMove)
		.def("is_root", &Node_::isRoot)
		.def("is_leaf", &Node_::isLeaf)
		.def("get_n_children", &Node_::getNChildren)
		.def("get_n_visits", &Node_::getNVisits)
		.def("get_accumulated_value", &Node_::getAccumulatedValue)
		.def("get_child", &Node_::getChild, p::return_value_policy<p::reference_existing_object>())
		.def("get_parent", &Node_::getParent, p::return_value_policy<p::reference_existing_object>());


	p::class_<Search_, std::shared_ptr<Search_>, boost::noncopyable>("Search", p::init<const Game&, std::shared_ptr<Evaluator>, size_t, size_t>())
	.def("get_tree_root", &Search_::getTreeRoot, p::return_value_policy<p::reference_existing_object>())
	.def("done", &Search_::done);
	
	p::class_<SearchPool, shared_ptr<SearchPool>, boost::noncopyable>("SearchPool", p::init<std::shared_ptr<Evaluator>, size_t>())
		.def("perform_search", &perform_search);

}
