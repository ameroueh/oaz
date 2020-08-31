#ifndef __TASK_HPP__
#define __TASK_HPP__

namespace oaz::thread_pool {
	class Task {
		public:
			virtual void operator()() = 0;
			virtual ~Task(){}
	};
}
#endif

