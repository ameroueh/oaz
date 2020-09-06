#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#ifndef TEST_FRIENDS
#define TEST_FRIENDS
#endif

#include <exception>
#include <random>
#include <vector>

#include "stdint.h"
#include "boost/multi_array.hpp"

#include "oaz/queue/queue.hpp"
#include "oaz/mutex/mutex.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"
#include "oaz/thread_pool/thread_pool.hpp"
#include "oaz/evaluator/evaluator.hpp"


namespace oaz::mcts {

	template <class Game, class Selector>
	class Search {

		TEST_FRIENDS;
		
		public:
	
			using Move = typename Game::Move;
			using Node = SearchNode<Move>;
			using Queue = oaz::queue::SafeQueue<size_t>;
			using Value = typename Game::Value;
			using Policy = typename Game::Policy;

			Search(
				const Game&, 
				oaz::evaluator::Evaluator<Game>*, 
				oaz::thread_pool::ThreadPool*, 
				size_t, 
				size_t
			);
			Search(
				const Game&, 
				oaz::evaluator::Evaluator<Game>*, 
				oaz::thread_pool::ThreadPool*, 
				size_t, 
				size_t, 
				float, 
				float
			);
			
			bool done() const;
			void search();
			void seedRNG(size_t);

			Move getBestMove();
			void getVisitCounts(Policy&);
			Node* getTreeRoot();
			~Search();
		
		private:
			class SelectionTask : public oaz::thread_pool::Task {
				public:
					SelectionTask(Search<Game, Selector>*, size_t);
					SelectionTask();
					void operator()();					
					~SelectionTask();
				private:
					Search<Game, Selector>* m_search;
					size_t m_index;
			};
			
			class ExpansionAndBackpropagationTask : public oaz::thread_pool::Task {
				public:
					ExpansionAndBackpropagationTask(Search<Game, Selector>*, size_t);
					ExpansionAndBackpropagationTask();
					void operator()();
					virtual ~ExpansionAndBackpropagationTask();
				private:
					Search<Game, Selector>* m_search;
					size_t m_index;
			};

			void handleFinishedTask();
			void handleCreatedTask();
			
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

			oaz::evaluator::Evaluator<Game>* m_evaluator;

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

			size_t m_n_selections;
			size_t m_n_iterations;

			std::atomic<size_t> m_n_completions;
			std::atomic<size_t> m_n_evaluation_requests;
			std::atomic<size_t> m_n_active_tasks;

			void incrementNCompletions();

			size_t getNSelections() const;
			size_t getNIterations() const; 
			size_t getNCompletions() const;
			size_t getNActiveTasks() const;

			size_t getEvaluatorIndex(size_t) const;

			void addDirichletNoise(Policy&);

			float m_noise_epsilon;
			float m_noise_alpha;

			oaz::thread_pool::ThreadPool* m_thread_pool;
			std::condition_variable m_condition;
			std::mutex m_mutex;

			boost::multi_array<SelectionTask, 1> m_selection_tasks;
			boost::multi_array<ExpansionAndBackpropagationTask, 1> m_expansion_and_backpropagation_tasks;
	};

}

#include "oaz/mcts/search_impl.cpp"
#endif
