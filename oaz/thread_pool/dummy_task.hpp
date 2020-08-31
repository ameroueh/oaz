#ifndef __DUMMY_TASK_HPP__
#define __DUMMY_TASK_HPP__

#include <mutex>
#include <condition_variable>

namespace oaz::thread_pool {
	class DummyTask : public oaz::thread_pool::Task {
		public:
			DummyTask(): m_executions(0), m_target(1) {}
			DummyTask(size_t target): m_executions(0), m_target(target) {}
			void operator()(){
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					++m_executions;
				}
				if(m_executions == m_target)
					m_condition.notify_one();
			}
			void wait() {
				std::unique_lock<std::mutex> lock(m_mutex);
				m_condition.wait(lock, [this]{return m_executions == m_target;});
			}	

		private:
			size_t m_executions;
			size_t m_target;
			std::mutex m_mutex;
			std::condition_variable m_condition;
	};
}
#endif
