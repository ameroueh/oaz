#include "Python.h"
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include "oaz/mcts/search.hpp"

namespace p = boost::python;

BOOST_PYTHON_MODULE( search ) {

	PyEval_InitThreads();

	p::class_<
		oaz::mcts::SearchNode,
		std::shared_ptr<oaz::mcts::SearchNode>, 
		boost::noncopyable>(
		"SearchNode",
		p::init<>()
	)
	.add_property("move", &SearchNode::GetMove)
	.add_property("is_root", &SearchNode::IsRoot)
	.add_property("is_leaf", &SearchNode::IsLeaf)
	.add_property("n_children", &SearchNode::GetNChildren)
	.add_property("n_visits", &SearchNode::GetNVisits)
	.add_property("accumulated_value", &SearchNode::GetAccumulatedValue)
	.def(
		"get_child", 
		&SearchNode::GetChild,
		p::return_value_policy<p::reference_existing_object>()
	)
	.def(
		"get_parent",
		&SearchNode::GetParent,
		p::return_value_policy<p::reference_existing_object>()
	);

	p::class_<Search, std::shared_ptr<oaz::mcts::Search>, boost::noncopyable>(
		"Search", 
		p::init<
			const oaz::games::Game&, 
			const oaz::mcts::Selector&,
			std::shared_ptr<oaz::evaluator::Evaluator>,
			std::shared_ptr<oaz::thread_pool::ThreadPool>,
			size_t,
			size_t,
			float,
			float
		>()
	)
	.def("get_tree_root", &Search::GetTreeRoot);
}
