#include "oaz/mutex/mutex.hpp"

#include <iostream>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace oaz::mutex;
using namespace testing;
using namespace std;

TEST(InstantiationTest, Default) { SpinlockMutex m; }

TEST(LockTest, Default) {
  SpinlockMutex m;
  m.Lock();
}

TEST(LockUnlockTest, Default) {
  SpinlockMutex m;
  m.Lock();
  m.Unlock();
}

void addOne(int* var, SpinlockMutex* mutex, int n_iterations) {
  for (int i = 0; i != n_iterations; ++i) {
    mutex->Lock();
    *var += 1;
    mutex->Unlock();
  }
}

TEST(ConcurrentLockTest, Default) {
  int counter = 0;
  int n_iterations = 10000;
  SpinlockMutex mutex;
  std::thread first(addOne, &counter, &mutex, n_iterations);
  std::thread second(addOne, &counter, &mutex, n_iterations);
  first.join();
  second.join();

  ASSERT_EQ(2 * n_iterations, counter);
}
