#include "oaz/thread_pool/thread_pool.hpp"

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::thread_pool;
using namespace testing;
using namespace std;

class TestTask : public Task {
 public:
  TestTask(std::vector<int>* pvector, std::mutex* pmutex,
           std::condition_variable* pcondition, int index)
      : m_pvector(pvector),
        m_pmutex(pmutex),
        m_pcondition(pcondition),
        m_index(index) {}

  void operator()() {
    {
      std::unique_lock<std::mutex> lock(*m_pmutex);
      m_pvector->push_back(m_index);
    }
    m_pcondition->notify_one();
  };

  ~TestTask() {}

 private:
  std::vector<int>* m_pvector;
  int m_index;
  std::mutex* m_pmutex;
  std::condition_variable* m_pcondition;
};

TEST(InstantiationTest, Default) { ThreadPool pool(1); }

TEST(PerformTask, Default) {
  ThreadPool pool(1);

  std::vector<int> vec;
  std::mutex mutex;
  std::condition_variable condition;

  TestTask task(&vec, &mutex, &condition, 0);
  pool.enqueue(&task);

  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock, [&vec] { return vec.size() > 0; });
  ASSERT_THAT(vec, ElementsAre(0));
}

TEST(PerformTask, Performance) {
  ThreadPool pool(1);

  std::vector<int> vec;
  std::mutex mutex;
  std::condition_variable condition;

  std::vector<std::unique_ptr<TestTask>> tasks;

  for (size_t i = 0; i != 100000; ++i) {
    tasks.push_back(
        std::unique_ptr<TestTask>(new TestTask(&vec, &mutex, &condition, i)));
    pool.enqueue(tasks[i].get());
  }

  std::unique_lock<std::mutex> lock(mutex);
  condition.wait(lock, [&vec] { return vec.size() == 100000; });
}
