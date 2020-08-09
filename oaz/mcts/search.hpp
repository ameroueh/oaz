#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <exception>
#include <random>
#include <vector>

#include "stdint.h"
#include "oaz/queue/queue.hpp"
#include "oaz/mutex/mutex.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"


namespace oaz::mcts {

	class SafeQueueNotifier {
		public:
			SafeQueueNotifier();
			SafeQueueNotifier(oaz::queue::SafeQueue<size_t>*, size_t);
			void operator()();
		private:
			oaz::queue::SafeQueue<size_t>* m_queue;
			size_t m_index;


	};

	template <class Game, class Evaluator, class Selector>
	class Search {

		TEST_FRIENDS;
		
		public:
			using Move = typename Game::Move;
			using Node = SearchNode<Move>;
			using Queue = oaz::queue::SafeQueue<size_t>;
			using Value = typename Game::Value;
			using Policy = typename Game::Policy;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;

			Search(const Game&, SharedEvaluatorPointer, size_t, size_t);
			Search(const Game&, SharedEvaluatorPointer, size_t, size_t, float, float);
			
			bool done() const;
			bool work();
			bool waitingForEvaluation();
			void seedRNG(size_t);

			Move getBestMove();
			void getVisitCounts(Policy&);
			Node* getTreeRoot();
			~Search();

		private:
			void selectNode(size_t); 
			void expandNode(Node* node, Game&, Policy&);
			Node* backpropagateNode(Node*, Game&, float); 
			void expandAndBackpropagateNode(size_t);
			void maybeSelect(size_t);
			void pause(size_t);
			void unpause(Node*);
			void enqueueForExpansionAndBackpropagation(size_t);
			
			size_t m_batch_size;
			Node m_root;
			
			Queue m_selection_q;
			Queue m_expansion_and_backpropagation_q;

			SharedEvaluatorPointer m_evaluator;

			std::vector<Node*> m_nodes;
			std::vector<Game> m_games;
			std::vector<typename Game::Value> m_values;
			std::vector<typename Game::Policy> m_policies;

			Node* getNode(size_t); 

			Game& getGame(size_t);
			Value& getValue(size_t);
			Policy& getPolicy(size_t);

			size_t getBatchSize() const;
			void initialise(const Game&);
			void deinitialise();

			void setNode(size_t, Node*);

			std::mt19937 m_generator; // Check if thread safe

			std::vector<Node*> m_paused_nodes;

			oaz::mutex::SpinlockMutex m_selection_lock;
			oaz::mutex::SpinlockMutex m_completion_lock;
			oaz::mutex::SpinlockMutex m_paused_nodes_lock;

			size_t m_n_selections;
			size_t m_n_iterations;
			size_t m_n_completions;
			size_t m_n_paused_nodes;

			std::atomic<size_t> m_n_evaluation_requests;

			void incrementNCompletions();

			size_t getNSelections() const;
			size_t getNIterations() const; 
			size_t getNCompletions() const;

			size_t getEvaluatorIndex(size_t) const;

			void addDirichletNoise(Policy&);

			float m_noise_epsilon;
			float m_noise_alpha;
	};

}

#include "oaz/mcts/search_impl.cpp"
#endif
