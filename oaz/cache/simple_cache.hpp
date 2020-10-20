#ifndef __SIMPLE_CACHE_HPP__
#define __SIMPLE_CACHE_HPP__

#include <atomic>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "stdint.h"

#include "boost/multi_array.hpp"

#include <iostream>

#include "oaz/cache/cache.hpp"
#include "oaz/games/game.hpp"

namespace oaz::cache {

	class SimpleCache : public Cache {
		public:
			SimpleCache(
				const oaz::games::Game& game,
				size_t size
			):
				m_size(size),
				m_n_hits(0),
				m_object_counter(0),
				m_policy_size(game.ClassMethods().GetMaxNumberOfMoves()),
				m_values(boost::extents[size]),
				m_policies(boost::extents[size][game.ClassMethods().GetMaxNumberOfMoves()]),
				m_map(game.ClassMethods().CreateGameMap()) {}

			bool Evaluate(
				const oaz::games::Game& game, 
				float& value,
				boost::multi_array_ref<float, 1> policy) {
				size_t object_id;
				{
					std::shared_lock<std::shared_mutex> l(m_shared_mutex);
					bool success = m_map->Get(game, object_id);
					if(!success) 
						return false;
				}
				IncrementNumberOfHits();
				value = m_values[object_id];
				policy = m_policies[object_id];
				return true;
			}
			void Insert(
				const oaz::games::Game& game, 
				float value,
				boost::multi_array_ref<float, 1> policy) {
				if(GetNumberOfObjects() >= GetSize())
					return;
				size_t object_id;
				{
					std::unique_lock<std::shared_mutex> l(m_shared_mutex);
					bool success = m_map->Get(game, object_id);  
					if(success)
						return;
					object_id = GetObjectID();
					m_map->Insert(game, object_id);
					m_values[object_id] = value;
					m_policies[object_id] = policy;
				}
			}

			void BatchInsert(
				boost::multi_array_ref<oaz::games::Game*, 1> games,
				boost::multi_array_ref<float*, 1> values,
				boost::multi_array_ref<std::unique_ptr<boost::multi_array_ref<float, 1>>,1> policies,
				size_t n_elements
			) {
				std::unique_lock<std::shared_mutex> l(m_shared_mutex);
				for(size_t i=0; i!=n_elements; ++i) {
					const oaz::games::Game& game = *(games[i]);
					if(GetNumberOfObjects() > GetSize())
						return;
					size_t object_id;
					if(m_map->Get(game, object_id))
						continue;
					object_id = GetObjectID();
					m_map->Insert(game, object_id);
					m_values[object_id] = *(values[i]);
					m_policies[object_id] = *(policies[i]);
				}
			}

			size_t GetNumberOfHits() const {
				return m_n_hits;
			}

			size_t GetSize() const {
				return m_size;
			}

			size_t GetNumberOfObjects() const {
				return m_object_counter;
			}

		private:
			size_t GetObjectID() {
				return m_object_counter++;
			}
			void IncrementNumberOfHits() {
				++m_n_hits;
			}
			std::atomic<size_t> m_object_counter;
			std::shared_mutex m_shared_mutex;

			boost::multi_array<float, 1> m_values;
			boost::multi_array<float, 2> m_policies;

			size_t m_size;
			size_t m_policy_size;
			std::atomic<size_t> m_n_hits;
			
			std::unique_ptr<oaz::games::Game::GameMap> m_map;
	};
}
#endif
