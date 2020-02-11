#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include <queue>
#include "oaz/mutex/mutex.hpp"

namespace oaz::queue {
	template <class T>
	class SafeQueue : public std::queue<T> {
		public:
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
}
#endif
