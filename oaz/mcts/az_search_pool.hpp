#ifndef __AZ_SEARCH_POOL_HPP__
#define __AZ_SEARCH_POOL_HPP__

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <memory>
#include <unordered_set>

#include "oaz/mcts/az_search.hpp"
#include "oaz/queue/queue.hpp"
#include "oaz/mutex/mutex.hpp"

namespace oaz::mcts {
	template <class Game, class Evaluator>
	class SearchContext {
		public:
			using Search = AZSearch<Game, Evaluator>;

			Search* getSearch();

			SearchContext(Search* search, std::condition_variable*, bool* status, std::mutex* mutex);
			~SearchContext();

		private:
			Search* m_search;
			bool* m_status;
			std::condition_variable* m_condition_variable;
			std::mutex* m_mutex;
	};

	template <class Game, class Evaluator>
	class AZSearchPool {
		// TO DO Should return instantiated search objects
		public:
			using Search = AZSearch<Game, Evaluator>;
			using SharedSearchPointer = std::shared_ptr<Search>;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
			using SharedSearchContextPointer = std::shared_ptr<SearchContext<Game, Evaluator>>;

			AZSearchPool(SharedEvaluatorPointer, size_t); // OK
			void performSearch(Search*); // OK
			/* void performSearch(SharedSearchPointer); // OK */

			std::string getStatus() const;

			~AZSearchPool();

		private:
			oaz::queue::SafeDeque<SharedSearchContextPointer> m_search_contexts_dq;
			SharedEvaluatorPointer m_evaluator;

			size_t getNSearches() const;
			size_t getNSearchesMaybeWaitingForEvaluation() const;
			/* size_t getNActiveWorkers() const; */
			/* size_t getNRequiredWorkers() const; */
			/* void updateNRequiredWorkers(); // OK */
			/* void maybeForceEvaluation(); */
			bool maybeStopWorking();
			void work(size_t);
			/* void maybeAddWorkers(); */
			void addSearch(Search*, std::condition_variable*, bool*, std::mutex*);
			void incrementNSearches();
			void decrementNSearches();
			/* void incrementNActiveWorkers(size_t); */
			/* void decrementNActiveWorkers(); */

			/* size_t m_n_active_workers; */
			size_t m_n_workers;
			/* size_t m_n_required_workers; */
			std::atomic<size_t> m_n_searches;

			oaz::mutex::SpinlockMutex m_workers_lock;
			/* oaz::mutex::SpinlockMutex m_waiting_searches_lock; */
			/* std::unordered_set<Search*> m_waiting_searches; */
			std::vector<std::thread> m_workers;
			bool m_maybe_stop_working;
			std::vector<size_t> m_n_iterations;

	};
}
#include "oaz/mcts/az_search_pool_impl.cpp"
#endif
