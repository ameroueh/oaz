#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

namespace oaz::logging {

	template <class SelfPlay, class Trainer, class Evaluator, class SearchPool>
	class Monitor {
		public:
			using SharedSelfPlayPointer = std::shared_ptr<SelfPlay>;
			using SharedTrainerPointer = std::shared_ptr<Trainer>;
			using SharedEvaluatorPointer = std::shared_ptr<Evaluator>;
			using SharedSearchPoolPointer = std::shared_ptr<SearchPool>;
			Monitor(
				SharedSelfPlayPointer self_play, 
				SharedTrainerPointer trainer, 
				SharedEvaluatorPointer evaluator,
				SharedSearchPoolPointer search_pool): 
				m_self_play(self_play),
				m_trainer(trainer),
				m_evaluator(evaluator),
				m_search_pool(search_pool) {

				std::future<void> future_exit_signal = m_exit_signal.get_future();
				m_worker = std::thread(	
					&Monitor::monitor,
					this,
					std::move(future_exit_signal)
				);
			}

			~Monitor() {
				m_exit_signal.set_value();
				m_worker.join();
			}
			

		private:
			
			void monitor(std::future<void> future_exit_signal) {
				while(future_exit_signal.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
					if(m_self_play)
						std::cout << m_self_play->getStatus() << std::endl;
					if(m_trainer)
						std::cout << m_trainer->getStatus() << std::endl;
					if(m_evaluator)
						std::cout << m_evaluator->getStatus() << std::endl;
					if(m_search_pool)
						std::cout << m_search_pool->getStatus() << std::endl;
				}
			}
			SharedSelfPlayPointer m_self_play;
			SharedTrainerPointer m_trainer;
			SharedEvaluatorPointer m_evaluator;
			SharedSearchPoolPointer m_search_pool;

			std::thread m_worker;
			std::promise<void> m_exit_signal;
	};
}
