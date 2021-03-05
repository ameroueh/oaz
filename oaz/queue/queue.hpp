#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include <deque>
#include <queue>

#include "oaz/mutex/mutex.hpp"

namespace oaz::queue {
template <class T>
class SafeQueue : public std::queue<T> {
 public:
  void Lock() { m_mutex.Lock(); }
  void Unlock() { m_mutex.Unlock(); }
  SafeQueue<T>& operator=(const SafeQueue<T>& rhs) {
    this->std::queue<T>::operator=(rhs);
    return *this;
  }

 private:
  oaz::mutex::SpinlockMutex m_mutex;
};

template <class T>
class SafeDeque : public std::deque<T> {
 public:
  void Lock() { m_mutex.Lock(); }
  void Unlock() { m_mutex.Unlock(); }
  SafeDeque<T>& operator=(const SafeDeque<T>& rhs) {
    this->std::deque<T>::operator=(rhs);
    return *this;
  }

 private:
  oaz::mutex::SpinlockMutex m_mutex;
};
}  // namespace oaz::queue
#endif
