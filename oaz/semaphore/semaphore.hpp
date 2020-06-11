#ifndef __SEMAPHORE_HPP__
#define __SEMAPHORE_HPP__

#include <atomic>

namespace oaz::semaphore {
	class SpinlockSemaphore {
		public:
			SpinlockSemaphore(int count): m_count(count) {}
			void lock() {
				bool success = false;
				while(!success) {
					int count = m_count;
					if( count - 1 >= 0 ) {
						success = std::atomic_compare_exchange_weak(
							&m_count,
							&count,
							count - 1
						);
					}
				}
			}
			void unlock() {
				++m_count;
			}
		
		private:
			std::atomic<int> m_count;
	};
}
#endif
