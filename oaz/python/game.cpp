#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
/* #include <boost/python/numpy.hpp> */

#include "oaz/games/connect_four.hpp"
#include "oaz/neural_network/model.hpp"
#include "oaz/neural_network/nn_evaluator.hpp"
#include "oaz/mcts/az_search.hpp"
#include "oaz/mcts/az_search_pool.hpp"

namespace p = boost::python;
/* using namespace np = boost::python::numpy; */

using Game = ConnectFour;
using Model = oaz::nn::Model;
using Evaluator = oaz::nn::NNEvaluator<Game, oaz::mcts::SafeQueueNotifier>;
using Search_ = oaz::mcts::AZSearch<Game, Evaluator>;
using SearchPool = oaz::mcts::AZSearchPool<Game, Evaluator>;
using Node = oaz::mcts::SearchNode<Game::Move>;

BOOST_PYTHON_MODULE(az_connect_four) {

	/* np::initialize(); */

	p::class_<Game>("ConnectFour")
		.def("play_move", &Game::playMove)
		.def("undo_move", &Game::undoMove)
		.def("current_player", &Game::getCurrentPlayer)
		.def("finished", &Game::Finished)
		.def("score", &Game::score)
		.def("get_policy_size", &Game::getPolicySize);
	
	p::class_<Model, std::shared_ptr<Model>, boost::noncopyable>("Model", p::init<>() )
		.def("create", &Model::create)
		.staticmethod("create")
		.def("load", &Model::Load)
		.def("get_value_node_name", &Model::getValueNodeName)
		.def("get_policy_node_name", &Model::getPolicyNodeName);
	
	p::class_<Evaluator, std::shared_ptr<Evaluator>, boost::noncopyable>("Evaluator", p::init<std::shared_ptr<Model>, size_t>());

	p::class_<Node, boost::noncopyable>("Node", p::init<>())
		.def("get_move", &Node::getMove)
		.def("is_root", &Node::isRoot)
		.def("is_leaf", &Node::isLeaf)
		.def("get_n_children", &Node::getNChildren)
		.def("get_n_visits", &Node::getNVisits)
		.def("get_accumulated_value", &Node::getAccumulatedValue)
		.def("get_child", &Node::getChild, p::return_value_policy<p::reference_existing_object>())
		.def("get_parent", &Node::getParent, p::return_value_policy<p::reference_existing_object>());


	p::class_<Search_, std::shared_ptr<Search_>, boost::noncopyable>("Search", p::init<const Game&, std::shared_ptr<Evaluator>, size_t, size_t>())
	.def("get_tree_root", &Search_::getTreeRoot, p::return_value_policy<p::reference_existing_object>())
	.def("done", &Search_::done);
	
	p::class_<SearchPool, shared_ptr<SearchPool>, boost::noncopyable>("SearchPool", p::init<std::shared_ptr<Evaluator>, size_t>())
		.def("perform_search", &SearchPool::performSearch);

}
