#ifndef OAZ_THREAD_POOL_THREAD_POOL_HPP_
#define OAZ_THREAD_POOL_THREAD_POOL_HPP_

// See COPYING for original license

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "oaz/thread_pool/task.hpp"

namespace oaz::thread_pool {
class ThreadPool {
 public:
  explicit ThreadPool(size_t n_threads);
  void enqueue(oaz::thread_pool::Task* task);
  ~ThreadPool();
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

 private:
  std::vector<std::thread> workers;
  std::queue<oaz::thread_pool::Task*> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

inline ThreadPool::ThreadPool(size_t n_threads) : stop(false) {
  for (size_t i = 0; i < n_threads; ++i) {
    workers.emplace_back([this] {
      for (;;) {
        oaz::thread_pool::Task* task = nullptr;

        {
          std::unique_lock<std::mutex> lock(this->queue_mutex);
          this->condition.wait(
              lock, [this] { return this->stop || !this->tasks.empty(); });
          if (this->stop && this->tasks.empty()) {return;}
          task = this->tasks.front();
          this->tasks.pop();
        }
        (*task)();
      }
    });
  }
}

inline void ThreadPool::enqueue(oaz::thread_pool::Task* task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);

    if (stop) {throw std::runtime_error("enqueue on stopped ThreadPool");}

    tasks.emplace(task);
  }
  condition.notify_one();
}

inline ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (std::thread& worker : workers) {worker.join();}
}
}  // namespace oaz::thread_pool
#endif  // OAZ_THREAD_POOL_THREAD_POOL_HPP_
