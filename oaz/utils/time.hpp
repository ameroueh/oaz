#ifndef __TIME_HPP__
#define __TIME_HPP__

#include <chrono>

namespace oaz::utils {

size_t time_now_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}
}  // namespace oaz::utils
#endif
