#include <algorithm>
#include <iostream>

#include "oaz/mcts/az_search_pool.hpp"

template <class Game, class Evaluator>
SearchContext<Game, Evaluator>::SearchContext(
	Search* search, 
	std::condition_variable* condition_variable,
	bool* status,
	std::mutex* mutex):
	m_search(search),
	m_condition_variable(condition_variable),
	m_status(status),
	m_mutex(mutex) {}

template <class Game, class Evaluator>
AZSearch<Game, Evaluator>* SearchContext<Game, Evaluator>::getSearch() {
	return m_search;
}

template <class Game, class Evaluator>
SearchContext<Game, Evaluator>::~SearchContext() {
	{
		std::lock_guard<std::mutex> lock(*m_mutex);
		*m_status = true;
	}
	m_condition_variable->notify_one();
}

template <class Game, class Evaluator>
AZSearchPool<Game, Evaluator>::AZSearchPool(SharedEvaluatorPointer evaluator, float n_workers_per_search):
	m_evaluator(evaluator),
	m_n_workers_per_search(n_workers_per_search),
	m_n_active_workers(0),
	m_n_required_workers(0),
	m_n_searches(0) {
}


template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::performSearch(Search* search) {

	std::condition_variable cond_variable;
	bool status = false;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	lock.unlock();
	
	incrementNSearches();
	addSearch(search, &cond_variable, &status, &mutex);	
	updateNRequiredWorkers();
	maybeAddWorkers();
	
	lock.lock();
	while(!status) {
		cond_variable.wait(lock);
	}
	decrementNSearches();
	updateNRequiredWorkers();
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::addSearch(
	Search* search, 
	std::condition_variable* cond_variable, 
	bool* status,
	std::mutex* mutex) {
	
	SharedSearchContextPointer shared_search_context_ptr(
		new SearchContext<Game, Evaluator>(search, cond_variable, status, mutex)
	);

	m_search_contexts_dq.lock();
	m_search_contexts_dq.push_back(shared_search_context_ptr);
	m_search_contexts_dq.unlock();
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::incrementNSearches() {
	m_n_searches++;
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::decrementNSearches() {
	m_n_searches--;
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::updateNRequiredWorkers() {
	if(getNSearches() == 0) 
		m_n_required_workers = 0;
	else 
		m_n_required_workers = static_cast<size_t>(
			std::max(
				static_cast<float>(m_n_searches)*m_n_workers_per_search,
				1.0f
			)
		);
}

template <class Game, class Evaluator>
size_t AZSearchPool<Game, Evaluator>::getNRequiredWorkers() const {
	return m_n_required_workers;
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::incrementNActiveWorkers(size_t n_workers) {
	m_n_active_workers += n_workers;
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::decrementNActiveWorkers() {
	m_n_active_workers--;
}

template <class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::maybeAddWorkers() {
	m_workers_lock.lock();
	size_t n_extra_workers = getNRequiredWorkers() - getNActiveWorkers();
	if(n_extra_workers > 0)
		incrementNActiveWorkers(n_extra_workers);
	m_workers_lock.unlock();
	
	for(size_t i=0; i!= n_extra_workers; ++i) {
		m_workers_lock.lock();
		m_workers.push_back(std::thread(&AZSearchPool<Game, Evaluator>::work, this));
		m_workers_lock.unlock();
	}
}

template<class Game, class Evaluator>
void AZSearchPool<Game, Evaluator>::work() {
	while(!maybeStopWorking()) {
		m_search_contexts_dq.lock();
		if (!m_search_contexts_dq.empty()) {
			SharedSearchContextPointer shared_search_context_ptr = m_search_contexts_dq.front();
			m_search_contexts_dq.pop_front();
			Search* search = shared_search_context_ptr->getSearch();
			if(!search->done()) {
				m_search_contexts_dq.push_back(shared_search_context_ptr);
				m_search_contexts_dq.unlock();
				if(!search->waitingForEvaluation())
					search->work();
				/* m_waiting_searches_lock.lock(); */
				/* if(search->waitingForEvaluation()) { */
				/* 	m_waiting_searches.insert(search); */	
				/* 	maybeForceEvaluation(); */
				/* } else { */
				/* 	m_waiting_searches.erase(search); */
				/* 	m_waiting_searches_lock.unlock(); */
				/* 	search->work(); */
				/* } */
			} else m_search_contexts_dq.unlock(); 
		} else m_search_contexts_dq.unlock();
	}
}

template <class Game, class Evaluator>
bool AZSearchPool<Game, Evaluator>::maybeStopWorking() {
	
	m_workers_lock.lock();
	if(getNRequiredWorkers() < getNActiveWorkers()) {
		decrementNActiveWorkers();
		m_workers_lock.unlock();
		return true;
	}
	else m_workers_lock.unlock();
	return false;
}

/* template <class Game, class Evaluator> */
/* void AZSearchPool<Game, Evaluator>::maybeForceEvaluation() { */
/* 	if(m_waiting_searches.size() >= getNSearches()) { */
/* 		m_waiting_searches_lock.unlock(); */
/* 		m_evaluator->forceEvaluation(); */
/* 	} else m_waiting_searches_lock.unlock(); */
/* } */

template <class Game, class Evaluator>
size_t AZSearchPool<Game, Evaluator>::getNSearches() const {
	return m_n_searches;
}


template <class Game, class Evaluator>
std::string AZSearchPool<Game, Evaluator>::getStatus() const {
	return "Search pool status: " + std::to_string(m_n_active_workers) + " active workers";
}

template <class Game, class Evaluator>
size_t AZSearchPool<Game, Evaluator>::getNActiveWorkers() const {
	return m_n_active_workers;
}

template <class Game, class Evaluator>
AZSearchPool<Game, Evaluator>::~AZSearchPool() {
	for(size_t i=0; i!= m_workers.size(); ++i) {
		m_workers[i].join();
	}
}
