#ifndef OAZ_MCTS_SEARCH_HPP_
#define OAZ_MCTS_SEARCH_HPP_

#ifndef TEST_FRIENDS
  #define TEST_FRIENDS
#endif

#include <exception>
#include <memory>
#include <random>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/evaluator/evaluator.hpp"
#include "oaz/games/game.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"
#include "oaz/mutex/mutex.hpp"
#include "oaz/thread_pool/thread_pool.hpp"

namespace oaz::mcts {

class Search {
  TEST_FRIENDS;

 public:
  Search(const oaz::games::Game&, const oaz::mcts::Selector&,
         std::shared_ptr<oaz::evaluator::Evaluator>,
         std::shared_ptr<oaz::thread_pool::ThreadPool>, size_t, size_t);
  Search(const oaz::games::Game&, const oaz::mcts::Selector&,
         std::shared_ptr<oaz::evaluator::Evaluator>,
         std::shared_ptr<oaz::thread_pool::ThreadPool>, size_t, size_t, float,
         float);

  /* void seedRNG(size_t); */
  std::shared_ptr<SearchNode> GetTreeRoot();
  
  ~Search();
  Search(const Search&) = delete;
  Search(Search&&) = delete;
  Search& operator=(const Search&) = delete;
  Search& operator=(Search&&) = delete;

 private:
  static constexpr float EPS_THRESHOLD = 0.001;
  class SelectionTask : public oaz::thread_pool::Task {
   public:
    SelectionTask(Search*, size_t);
    SelectionTask();
    void operator()() override;

   private:
    Search* m_search;
    size_t m_index;
  };

  class ExpansionAndBackpropagationTask : public oaz::thread_pool::Task {
   public:
    ExpansionAndBackpropagationTask(Search*, size_t);
    ExpansionAndBackpropagationTask();
    void operator()() override;

   private:
    Search* m_search;
    size_t m_index;
  };

  size_t GetBatchSize() const;
  size_t GetNSelections() const;
  size_t GetNIterations() const;
  size_t GetNCompletions() const;
  size_t GetNActiveTasks() const;
  size_t GetEvaluatorIndex(size_t) const;
  size_t GetPolicySize() const;
  bool Done() const;

  oaz::games::Game* GetGame(size_t);
  void ResetGame(size_t);
  float& GetValue(size_t);
  boost::multi_array_ref<float, 1> GetPolicy(size_t);
  SearchNode* GetNode(size_t);

  void SetNode(size_t, SearchNode*);

  void IncrementNCompletions();

  void Initialise();
  void Deinitialise();

  void HandleFinishedTask();
  void HandleCreatedTask();

  void SelectNode(size_t);
  void ExpandNode(SearchNode* node, oaz::games::Game*,
                  boost::multi_array_ref<float, 1>);
  static void BackpropagateNode(SearchNode*, float);
  void ExpandAndBackpropagateNode(size_t);
  void MaybeSelect(size_t);
  void Pause(size_t);
  void Unpause(SearchNode*);

  void AddDirichletNoise(boost::multi_array_ref<float, 1>);
  void PerformSearch();

  size_t m_batch_size;
  size_t m_policy_size;

  std::vector<SearchNode*> m_nodes;
  std::vector<std::unique_ptr<oaz::games::Game>> m_games;

  boost::multi_array<float, 1> m_values;
  boost::multi_array<float, 2> m_policies;

  std::mt19937 m_generator;  // Check if thread safe

  std::vector<SearchNode*> m_paused_nodes;

  oaz::mutex::SpinlockMutex m_selection_lock;

  size_t m_n_selections;
  size_t m_n_iterations;

  std::atomic<size_t> m_n_completions;
  std::atomic<size_t> m_n_evaluation_requests;
  std::atomic<size_t> m_n_active_tasks;

  float m_noise_epsilon;
  float m_noise_alpha;

  std::condition_variable m_condition;
  std::mutex m_mutex;

  boost::multi_array<SelectionTask, 1> m_selection_tasks;
  boost::multi_array<ExpansionAndBackpropagationTask, 1>
      m_expansion_and_backpropagation_tasks;

  std::shared_ptr<oaz::thread_pool::ThreadPool> m_thread_pool;
  std::shared_ptr<oaz::evaluator::Evaluator> m_evaluator;
  std::shared_ptr<SearchNode> m_root;

  std::unique_ptr<oaz::games::Game> m_game;
  std::unique_ptr<Selector> m_selector;
};

}  // namespace oaz::mcts

#endif  // OAZ_MCTS_SEARCH_HPP_
