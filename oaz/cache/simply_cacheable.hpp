#ifndef __SIMPLY_CACHEABLE_HPP__
#define __SIMPLY_CACHEABLE_HPP__

#include "stdint.h"

namespace oaz::cache {
	class SimplyCacheable {
		public:
			virtual uint64_t GetStateAsUint64() const = 0;			
			virtual ~SimplyCacheable() {}
	};
}
#endif
