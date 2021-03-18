#ifndef OAZ_UTILS_TIME_HPP_
#define OAZ_UTILS_TIME_HPP_

#include <chrono>

namespace oaz::utils {

size_t time_now_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}
}  // namespace oaz::utils
#endif  // OAZ_UTILS_TIME_HPP_
