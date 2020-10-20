#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include "boost/multi_array.hpp"

#include "oaz/games/game.hpp"

namespace oaz::cache {
	class Cache {
		public:
			virtual bool Evaluate(
				const oaz::games::Game&,
				float&, 
				boost::multi_array_ref<float, 1>
			) = 0;
			virtual void Insert(
				const oaz::games::Game&,
				float, 
				boost::multi_array_ref<float, 1>
			) = 0;

			virtual void BatchInsert(
				boost::multi_array_ref<oaz::games::Game*, 1>,
				boost::multi_array_ref<float*, 1>,
				boost::multi_array_ref<std::unique_ptr<boost::multi_array_ref<float, 1>>, 1>,
				size_t
			) = 0;
			virtual ~Cache() {}
	};
}
#endif
