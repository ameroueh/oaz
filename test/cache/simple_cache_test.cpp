#include "oaz/cache/simple_cache.hpp"

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "oaz/games/tic_tac_toe.hpp"

using namespace oaz::cache;
using namespace testing;


class SimpleEvaluation : public oaz::evaluator::Evaluation {
  public:
    SimpleEvaluation(float value, float policy): m_value(value), m_policy(policy) {}
    float GetValue() const { return m_value; }
    float GetPolicy(size_t move) const { return m_policy; }
    std::unique_ptr<oaz::evaluator::Evaluation> Clone() const {
      return std::make_unique<SimpleEvaluation>(*this);
    }
  private:
    float m_value;
    float m_policy;
};

TEST(InstantiationTest, Default) {
  oaz::games::TicTacToe game;
  SimpleCache cache(game, 100);
}

TEST(Evaluate, NotInDB) {
  oaz::games::TicTacToe game;
  SimpleCache cache(game, 100);
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation = std::move(
    std::make_unique<SimpleEvaluation>(0, 0)
  );
  ASSERT_FALSE(cache.Evaluate(game, &evaluation));
}

TEST(Insert, Default) {
  oaz::games::TicTacToe game;
  SimpleCache cache(game, 100);
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation = std::move(
    std::make_unique<SimpleEvaluation>(0, 0)
  );
  cache.Insert(game, &evaluation);
  ASSERT_EQ(cache.GetNumberOfObjects(), 1);
}

TEST(Evaluate, InDB) {
  oaz::games::TicTacToe game;
  SimpleCache cache(game, 100);
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation = std::move(
    std::make_unique<SimpleEvaluation>(0.5F, 0.6F)
  );
  cache.Insert(game, &evaluation);
 
  std::unique_ptr<oaz::evaluator::Evaluation> evaluation2;
  cache.Evaluate(game, &evaluation2);

  ASSERT_EQ(0.5F, evaluation2->GetValue());
  ASSERT_EQ(0.6F, evaluation2->GetPolicy(0));
}

TEST(InstantiationTest, LargeInstance) {
  oaz::games::TicTacToe game;
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
/* 	boost::multi_array<float, 2>
 * read_values(boost::extents[N_THREADS][N_ITEMS]); */
/* 	boost::multi_array<float, 3>
 * read_policies(boost::extents[N_THREADS][N_ITEMS][10]); */

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
/* 	boost::multi_array<float, 2>
 * read_values(boost::extents[N_THREADS][N_ITEMS]); */
/* 	boost::multi_array<float, 3>
 * read_policies(boost::extents[N_THREADS][N_ITEMS][10]); */

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
