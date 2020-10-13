#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

#include <atomic>

namespace oaz::mutex {
	class SpinlockMutex {
		public:
			SpinlockMutex(): m_locked(false) {}
			void Lock() {
				bool expected = false;
				while( !std::atomic_compare_exchange_weak(&m_locked, &expected, true) ) {
					expected = false;				
				}
			}
			void Unlock() {
				m_locked = false;
			}
		
		private:
			std::atomic<bool> m_locked;
	};
}
#endif
