#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <random>
#include <vector>

#include "stdint.h"
#include "oaz/queue/queue.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"


namespace oaz::mcts {

	template <class Game, class GameEvaluator>
	class Search {

		TEST_FRIENDS;

		public:
			using evaluatorptr_t = GameEvaluator*;
			using gameptr_t = Game*;
			using move_t = typename Game::move_t;
			using node_t = SearchNode<move_t>;
			using nodeptr_t = node_t*;
			using tsize_t  = typename node_t::tsize_t;

			using queue_t = oaz::queue::SafeQueue<size_t>;
			using PolicyType = typename GameEvaluator::PolicyType;

			Search(tsize_t, const Game&, GameEvaluator*);

			void search(size_t);
			move_t getBestMove();
			nodeptr_t getTreeRoot();

		private:
			void selectNode(size_t); 
			void selectNodes();
			void evaluateNodes(); 
			void expandNode(nodeptr_t node, Game&, PolicyType&);
			nodeptr_t backpropagateNode(nodeptr_t, Game&, float); 
			void expandAndBackpropagateNode(size_t);
			void expandAndBackpropagateNodes();
			
			size_t m_batch_size;
			node_t m_root;
			
			queue_t m_selection_q;
			queue_t m_evaluation_q;
			queue_t m_reselection_q;

			GameEvaluator* m_game_evaluator;

			std::vector<nodeptr_t> m_nodes;

			nodeptr_t getNode(size_t); 

			size_t getBatchSize() const;
			void initialise(const Game&);
			void setNode(size_t, nodeptr_t);

			std::mt19937 m_generator; // check if thread safe
	};
}
#include "oaz/mcts/search_impl.cpp"
#endif
