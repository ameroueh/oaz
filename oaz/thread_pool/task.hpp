#ifndef OAZ_THREAD_POOL_TASK_HPP_
#define OAZ_THREAD_POOL_TASK_HPP_

namespace oaz::thread_pool {
class Task {
 public:
  virtual void operator()() = 0;
  virtual ~Task() {}
};
}  // namespace oaz::thread_pool
#endif  // OAZ_THREAD_POOL_TASK_HPP_
