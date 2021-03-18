#ifndef OAZ_CACHE_SIMPLY_CACHEABLE_HPP_
#define OAZ_CACHE_SIMPLY_CACHEABLE_HPP_

#include <stdint.h>

namespace oaz::cache {
class SimplyCacheable {
 public:
  virtual uint64_t GetStateAsUint64() const = 0;
  virtual ~SimplyCacheable() {}
};
}  // namespace oaz::cache
#endif  // OAZ_CACHE_SIMPLY_CACHEABLE_HPP_
