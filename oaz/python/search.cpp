#include "oaz/mcts/search.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"

namespace p = boost::python;

namespace oaz::mcts {
class SearchWrapper {
 public:
  SearchWrapper(const oaz::games::Game& game,
                const oaz::mcts::Selector& selector,
                std::shared_ptr<oaz::evaluator::Evaluator> evaluator,
                std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
                size_t batch_size, size_t n_iterations, float noise_epsilon,
                float noise_alpha

                )
      : m_search(nullptr) {
    PyThreadState* save_state = PyEval_SaveThread();
    m_search = std::make_shared<oaz::mcts::Search>(
        game, selector, evaluator, thread_pool, batch_size, n_iterations,
        noise_epsilon, noise_alpha);
    PyEval_RestoreThread(save_state);
  }
  std::shared_ptr<oaz::mcts::SearchNode> GetTreeRoot() {
    return m_search->GetTreeRoot();
  }

 private:
  std::shared_ptr<oaz::mcts::Search> m_search;
};
}  // namespace oaz::mcts

BOOST_PYTHON_MODULE(search) {
  PyEval_InitThreads();

  p::class_<oaz::mcts::SearchNode, std::shared_ptr<oaz::mcts::SearchNode>,
            boost::noncopyable>("SearchNode", p::init<>())
      .add_property("move", &oaz::mcts::SearchNode::GetMove)
      .add_property("is_root", &oaz::mcts::SearchNode::IsRoot)
      .add_property("is_leaf", &oaz::mcts::SearchNode::IsLeaf)
      .add_property("n_children", &oaz::mcts::SearchNode::GetNChildren)
      .add_property("n_visits", &oaz::mcts::SearchNode::GetNVisits)
      .add_property("accumulated_value",
                    &oaz::mcts::SearchNode::GetAccumulatedValue)
      .def("get_child", &oaz::mcts::SearchNode::GetChild,
           p::return_value_policy<p::reference_existing_object>())
      .def("get_parent", &oaz::mcts::SearchNode::GetParent,
           p::return_value_policy<p::reference_existing_object>());

  p::class_<oaz::mcts::SearchWrapper, std::shared_ptr<oaz::mcts::SearchWrapper>,
            boost::noncopyable>(
      "Search", p::init<const oaz::games::Game&, const oaz::mcts::Selector&,
                        std::shared_ptr<oaz::evaluator::Evaluator>,
                        std::shared_ptr<oaz::thread_pool::ThreadPool>, size_t,
                        size_t, float, float>())
      .def("get_tree_root", &oaz::mcts::SearchWrapper::GetTreeRoot);
}
