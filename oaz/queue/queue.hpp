#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include <queue>
#include <deque>
#include "oaz/mutex/mutex.hpp"

namespace oaz::queue {
	template <class T>
	class SafeQueue : public std::queue<T> { public:
			void lock() {
				m_mutex.lock();
			}
			void unlock() {
				m_mutex.unlock();
			}
			SafeQueue<T>& operator=(const SafeQueue<T>& rhs) {
				this->std::queue<T>::operator=(rhs);
				return *this;
			}

		private:
			oaz::mutex::SpinlockMutex m_mutex;
	};
	
	template <class T>
	class SafeDeque : public std::deque<T> { public:
			void lock() {
				m_mutex.lock();
			}
			void unlock() {
				m_mutex.unlock();
			}
			SafeDeque<T>& operator=(const SafeDeque<T>& rhs) {
				this->std::deque<T>::operator=(rhs);
				return *this;
			}

		private:
			oaz::mutex::SpinlockMutex m_mutex;
	};
}
#endif
