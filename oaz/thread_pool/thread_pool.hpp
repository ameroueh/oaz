#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "oaz/thread_pool/task.hpp"


namespace oaz::thread_pool {
	class ThreadPool {
	public:
	    ThreadPool(size_t);
	    void enqueue(Task*);
	    ~ThreadPool();
	private:
	    std::vector< std::thread > workers;
	    std::queue< Task* > tasks;
	    std::mutex queue_mutex;
	    std::condition_variable condition;
	    bool stop;
	};
	 
	inline ThreadPool::ThreadPool(size_t threads)
	    :   stop(false)
	{
	    for(size_t i = 0;i<threads;++i)
		workers.emplace_back(
		    [this]
		    {
			for(;;)
			{
			    Task* task;

			    {
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->condition.wait(lock,
				    [this]{ return this->stop || !this->tasks.empty(); });
				if(this->stop && this->tasks.empty())
				    return;
				task = this->tasks.front();
				this->tasks.pop();
			    }
			    (*task)();
			}
		    }
		);
	}


	void ThreadPool::enqueue(Task* task) 
	{
	    {
		std::unique_lock<std::mutex> lock(queue_mutex);

		if(stop)
		    throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace(task);
	    }
	    condition.notify_one();
	}

	inline ThreadPool::~ThreadPool()
	{
	    {
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	    }
	    condition.notify_all();
	    for(std::thread &worker: workers)
		worker.join();
	}
}
#endif
