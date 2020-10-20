#include <algorithm>
#include <string>
#include <thread>
#include <vector>

#include "oaz/cache/simple_cache.hpp"
#include "oaz/games/tic_tac_toe.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace oaz::cache;
using namespace testing;


/* class DummySimplyCacheable : public SimplyCacheable { */
/* 	public: */
/* 		DummySimplyCacheable(size_t id): m_id(id) {} */
/* 		uint64_t GetStateAsUint64() const { */
/* 			return m_id; */
/* 		} */
/* 	private: */
/* 		size_t m_id; */
/* }; */

TEST (InstantiationTest, Default) {
	TicTacToe game;
	SimpleCache cache(game, 100);
}

TEST (Evaluate, NotInDB) {
	TicTacToe game;
	SimpleCache cache(game, 100);
	float value = 0;
	boost::multi_array<float, 1> policy(boost::extents[9]);
	ASSERT_FALSE(cache.Evaluate(game, value, policy));
}

TEST (Insert, Default) {
	TicTacToe game;
	SimpleCache cache(game, 100);
	float value = 0;
	boost::multi_array<float, 1> policy(boost::extents[9]);
	cache.Insert(game, value, policy);

	ASSERT_EQ(cache.GetNumberOfObjects(), 1);
}

TEST (Evaluate, InDB) {
	TicTacToe game;
	SimpleCache cache(game, 100);
	float value = 0;
	boost::multi_array<float, 1> policy(boost::extents[9]);
	for(size_t i=0; i!=9; ++i)
		policy[i] = 0.;
	cache.Insert(game, value, policy);

	float read_value = 1;
	boost::multi_array<float, 1> read_policy(boost::extents[9]);
	for(size_t i=0; i!=9; ++i)
		read_policy[i] = 1.;
	cache.Evaluate(game, read_value, read_policy);

	ASSERT_EQ(value, read_value);
	ASSERT_EQ(policy, read_policy);
}

TEST (InstantiationTest, LargeInstance) {
	TicTacToe game;
	SimpleCache cache(game, 5000000);
}

/* void QueryCache( */
/* 	SimpleCache* cache, */ 
/* 	boost::multi_array<float, 1>* values, */
/* 	boost::multi_array<float, 2>* policies, */
/* 	boost::multi_array<float, 2>* read_values, */
/* 	boost::multi_array<float, 3>* read_policies, */
/* 	size_t n_queries, */
/* 	size_t thread_id) { */
/* 	bool success = false; */
/* 	size_t cache_size = cache->GetSize(); */
/* 	for(size_t i=0; i!= n_queries; ++i) { */

/* 		size_t j = i % cache_size; */
/* 		DummySimplyCacheable cacheable(j); */
/* 		success = cache->Evaluate( */
/* 			cacheable, */ 
/* 			(*read_values)[thread_id][j], */
/* 			boost::multi_array_ref<float, 1>( */
/* 				(*read_policies)[thread_id][j].origin(), */
/* 				boost::extents[10] */
/* 			) */
/* 		); */
/* 		if(!success) */
/* 			cache->Insert( */
/* 				cacheable, */ 
/* 				(*values)[j], */ 
/* 				boost::multi_array_ref<float, 1>( */
/* 					(*policies)[j].origin(), */
/* 					boost::extents[10] */
/* 				) */
/* 			); */
/* 	} */
/* } */

/* TEST(MultithreadedAccess, Default) { */
/* 	size_t N_ITEMS = 1000; */
/* 	size_t N_THREADS = 4; */
/* 	size_t N_QUERIES_PER_THREAD=1000000; */

/* 	boost::multi_array<float, 1> values(boost::extents[N_ITEMS]); */
/* 	boost::multi_array<float, 2> policies(boost::extents[N_ITEMS][10]); */
/* 	boost::multi_array<float, 2> read_values(boost::extents[N_THREADS][N_ITEMS]); */
/* 	boost::multi_array<float, 3> read_policies(boost::extents[N_THREADS][N_ITEMS][10]); */

/* 	SimpleCache cache(10, N_ITEMS); */
	
/* 	std::vector<std::thread> workers; */
/* 	for(size_t i=0; i!=N_THREADS; ++i) { */
/* 		workers.push_back( */
/* 			std::thread( */
/* 				&QueryCache, */
/* 				&cache, */
/* 				&values, */
/* 				&policies, */
/* 				&read_values, */
/* 				&read_policies, */
/* 				N_QUERIES_PER_THREAD, */
/* 				i */
/* 			) */
/* 		); */
/* 	} */
/* 	for(size_t i=0; i!=N_THREADS; ++i) */
/* 		workers[i].join(); */
/* } */

/* TEST(MultithreadedAccess, SinglethreadHighSize) { */
/* 	size_t N_ITEMS = 1000000; */
/* 	size_t N_THREADS = 1; */
/* 	size_t N_QUERIES_PER_THREAD=1000000; */

/* 	boost::multi_array<float, 1> values(boost::extents[N_ITEMS]); */
/* 	boost::multi_array<float, 2> policies(boost::extents[N_ITEMS][10]); */
/* 	boost::multi_array<float, 2> read_values(boost::extents[N_THREADS][N_ITEMS]); */
/* 	boost::multi_array<float, 3> read_policies(boost::extents[N_THREADS][N_ITEMS][10]); */

/* 	SimpleCache cache(10, N_ITEMS); */
	
/* 	std::vector<std::thread> workers; */
/* 	for(size_t i=0; i!=N_THREADS; ++i) { */
/* 		workers.push_back( */
/* 			std::thread( */
/* 				&QueryCache, */
/* 				&cache, */
/* 				&values, */
/* 				&policies, */
/* 				&read_values, */
/* 				&read_policies, */
/* 				N_QUERIES_PER_THREAD, */
/* 				i */
/* 			) */
/* 		); */
/* 	} */
/* 	for(size_t i=0; i!=N_THREADS; ++i) */
/* 		workers[i].join(); */
/* } */
