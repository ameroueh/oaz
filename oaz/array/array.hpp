#ifndef OAZ_ARRAY_ARRAY_HPP_
#define OAZ_ARRAY_ARRAY_HPP_

#include <cstring>
#include <tuple>
#include <utility>

#include "boost/multi_array.hpp"

namespace oaz::array {

template <class none = void>
constexpr size_t accumulate_multiply() {
  return 1;
}

template <size_t i0, size_t... args>
constexpr size_t accumulate_multiply() {
  return i0 * accumulate_multiply<args...>();
}

template <size_t... dimensions>
class Array : public boost::multi_array<float, sizeof...(dimensions)> {
 public:
  static constexpr std::array<size_t, sizeof...(dimensions)> Dimensions() {
    return {dimensions...};
  }

  static constexpr size_t NumDimensions() { return sizeof...(dimensions); }

  static constexpr size_t NumElements() {
    return accumulate_multiply<dimensions...>();
  }

  static constexpr size_t SizeBytes() { return sizeof(float) * NumElements(); }

  Array()
      : boost::multi_array<float, sizeof...(dimensions)>(
            std::array<size_t, sizeof...(dimensions)>({dimensions...})) {}

  void Set(float* source) {
    std::memcpy(boost::multi_array<size_t, sizeof...(dimensions)>::origin(),
                source, SizeBytes());
  }
};
}  // namespace oaz::array

#endif // OAZ_ARRAY_ARRAY_HPP_
