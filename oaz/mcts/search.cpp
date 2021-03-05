#include "oaz/mcts/search.hpp"

#include <algorithm>
#include <exception>
#include <iostream>
#include <random>
#include <vector>

#include "oaz/mcts/backpropagation.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"
#include "stdint.h"

/* #include "spdlog/spdlog.h" */
#include "boost/multi_array.hpp"

using namespace oaz::mcts;

Search::SelectionTask::SelectionTask(Search* search, size_t index)
    : m_index(index), m_search(search) {
  m_search->HandleCreatedTask();
}

Search::SelectionTask::SelectionTask() : m_index(0), m_search(nullptr) {}

void Search::SelectionTask::operator()() {
  /* spdlog::debug("Search {:p} index {} selection task", (void*)m_search,
   * m_index); */
  m_search->SelectNode(m_index);
  m_search->HandleFinishedTask();
}

Search::SelectionTask::~SelectionTask() {}

Search::ExpansionAndBackpropagationTask::ExpansionAndBackpropagationTask(
    Search* search, size_t index)
    : m_index(index), m_search(search) {
  m_search->HandleCreatedTask();
}

Search::ExpansionAndBackpropagationTask::ExpansionAndBackpropagationTask()
    : m_index(0), m_search(nullptr) {}

void Search::ExpansionAndBackpropagationTask::operator()() {
  /* spdlog::debug("Search {:p} index {} expansion and backpropagation task",
   * (void*)m_search, m_index); */
  m_search->ExpandAndBackpropagateNode(m_index);
  m_search->HandleFinishedTask();
}

Search::ExpansionAndBackpropagationTask::~ExpansionAndBackpropagationTask() {}

void Search::SelectNode(size_t index) {
  /* spdlog::debug("Search {:p} index {} selectNode", (void*)this, index); */
  oaz::mcts::SearchNode* node = GetNode(index);
  oaz::games::Game* game = GetGame(index);

  while (true) {
    node->Lock();
    if (!node->IsLeaf()) {
      node->IncrementNVisits();
      node->Unlock();
      size_t child_index = (*m_selector)(node);
      node = node->GetChild(child_index);
      game->PlayMove(node->GetMove());
      SetNode(index, node);

    } else if (node->IsBlockedForEvaluation()) {
      /* spdlog::debug("Search {:p} index {} selectNode node {:p} blocked for
       * evaluation, pausing", (void*)this, index, (void*)node); */
      Pause(index);
      node->Unlock();
      break;
    } else {
      /* spdlog::debug("Search {:p} index {} selectNode node {:p} requesting
       * evaluation", (void*)this, index, (void*)node); */
      node->IncrementNVisits();
      if (!game->IsFinished()) {
        /* spdlog::debug("Search {:p} index {} selectNode node {:p} game not
         * finished, blocking evaluations", (void*)this, index, (void*)node); */
        node->BlockForEvaluation();
      }
      node->Unlock();
      m_n_evaluation_requests++;
      m_expansion_and_backpropagation_tasks[index] =
          ExpansionAndBackpropagationTask(this, index);

      m_evaluator->RequestEvaluation(
          game, &GetValue(index), GetPolicy(index),
          &m_expansion_and_backpropagation_tasks[index]);
      /* spdlog::debug("Search {:p} index {} selectNode node {:p} returning",
       * (void*)this, index, (void*)node); */
      break;
    }
  }
}

float& Search::GetValue(size_t index) { return m_values[index]; }

/*  */
/* void Search::SeedRNG(size_t seed) { */
/* 	m_generator.seed(seed); */
/* } */

boost::multi_array_ref<float, 1> Search::GetPolicy(size_t index) {
  return boost::multi_array_ref<float, 1>(m_policies[index].origin(),
                                          boost::extents[GetPolicySize()]);
}

void Search::Pause(size_t index) { m_paused_nodes[index] = GetNode(index); }

size_t Search::GetPolicySize() const { return m_policy_size; }

void Search::AddDirichletNoise(boost::multi_array_ref<float, 1> policy) {
  if (m_noise_epsilon > 0.001) {
    boost::multi_array<float, 1> noise(boost::extents[GetPolicySize()]);
    float total_noise = 0.;
    for (size_t i = 0; i != GetPolicySize(); ++i) {
      std::gamma_distribution<float> distribution(m_noise_alpha, 1.);
      noise[i] = distribution(m_generator);
      total_noise += noise[i];
    }
    for (size_t i = 0; i != GetPolicySize(); ++i) {
      policy[i] = (1 - m_noise_epsilon) * policy[i] +
                  m_noise_epsilon * noise[i] / total_noise;
    }
  }
}

void Search::ExpandNode(oaz::mcts::SearchNode* node, oaz::games::Game* game,
                        boost::multi_array_ref<float, 1> policy) {
  /* spdlog::debug("Search {:p} node {:p} expandNode", (void*)this,
   * (void*)node); */

  if (node->IsRoot()) AddDirichletNoise(policy);

  std::vector<size_t> available_moves;
  game->GetAvailableMoves(available_moves);
  size_t player = game->GetCurrentPlayer();

  for (auto move : available_moves) {
    float prior = policy[move];
    node->AddChild(move, player, prior);
  }
}

void Search::Unpause(oaz::mcts::SearchNode* node) {
  /* spdlog::debug("Search {:p} node {:p} unpause", (void*)this, (void*)node);
   */
  for (size_t index = 0; index != GetBatchSize(); ++index) {
    if (m_paused_nodes[index] == node) {
      m_selection_tasks[index] = SelectionTask(this, index);
      m_thread_pool->enqueue(&m_selection_tasks[index]);
      m_paused_nodes[index] = nullptr;
      /* spdlog::debug("Search {:p} node {:p} unpause unpaused index {}",
       * (void*)this, (void*)node, index); */
    }
  }
}

size_t Search::GetNSelections() const { return m_n_selections; }

size_t Search::GetNIterations() const { return m_n_iterations; }

void Search::MaybeSelect(size_t index) {
  /* spdlog::debug("Search {:p} index {} maybeSelect n_selections {} / {};
   * n_completions {} / {}", */
  /* (void*)this, */
  /* index, */
  /* GetNSelections(), */
  /* GetNIterations(), */
  /* GetNCompletions(), */
  /* GetNIterations() */
  /* ); */

  m_selection_lock.Lock();
  if (GetNSelections() < GetNIterations()) {
    /* spdlog::debug("Search {:p} index {} maybeSelect new selection",
     * (void*)this, index); */
    ++m_n_selections;
    m_selection_lock.Unlock();
    m_selection_tasks[index] = SelectionTask(this, index);
    m_thread_pool->enqueue(&m_selection_tasks[index]);
  } else
    m_selection_lock.Unlock();
  /* spdlog::debug("Search {:p} index {} maybeSelect returning", (void*)this,
   * index); */
}

oaz::games::Game* Search::GetGame(size_t index) { return m_games[index].get(); }

void Search::ExpandAndBackpropagateNode(size_t index) {
  /* spdlog::debug("Search {:p} index {} expandAndBackpropagate", (void*)this,
   * index); */

  float value = GetValue(index);
  value = (value + 1.) / 2.;

  boost::multi_array_ref<float, 1> policy = GetPolicy(index);
  oaz::mcts::SearchNode* node = GetNode(index);
  oaz::games::Game* game = GetGame(index);

  if (!game->IsFinished()) {
    /* spdlog::debug("Search {:p} index {} expandAndBackpropagate game not
     * finished", (void*)this, index); */
    node->Lock();
    ExpandNode(node, game, policy);
    node->UnblockForEvaluation();
    Unpause(node);
    node->Unlock();
  }

  BackpropagateNode(node, value);
  SetNode(index, m_root.get());
  IncrementNCompletions();
  ResetGame(index);
  MaybeSelect(index);
  /* spdlog::debug("Search {:p} index {} expandAndBackpropagate returning",
   * (void*)this, index); */
}

void Search::ResetGame(size_t index) {
  m_games[index] = std::move(m_game->Clone());
}

bool Search::Done() const {
  return (GetNCompletions() == GetNIterations()) && (GetNActiveTasks() == 0);
}

void Search::IncrementNCompletions() {
  ++m_n_completions;
  /* spdlog::debug("Search {:p} incrementNCompletions", (void*)this); */
}

size_t Search::GetNCompletions() const { return m_n_completions; }

void Search::Initialise() {
  std::random_device seeder;
  m_generator.seed(seeder());

  for (size_t index = 0; index != GetBatchSize(); ++index) {
    ResetGame(index);
    m_nodes[index] = m_root.get();
    m_paused_nodes[index] = nullptr;
  }
}

Search::Search(const oaz::games::Game& game, const Selector& selector,
               std::shared_ptr<oaz::evaluator::Evaluator> evaluator,
               std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
               size_t batch_size, size_t n_iterations)
    : m_root(std::make_shared<oaz::mcts::SearchNode>()),
      m_policy_size(game.ClassMethods().GetMaxNumberOfMoves()),
      m_game(std::move(game.Clone())),
      m_selector(std::move(selector.Clone())),
      m_batch_size(batch_size),
      m_n_iterations(n_iterations),
      m_n_selections(0),
      m_n_completions(0),
      m_n_evaluation_requests(0),
      m_n_active_tasks(0),
      m_nodes(batch_size),
      m_paused_nodes(batch_size),
      m_games(batch_size),
      m_values(boost::extents[batch_size]),
      m_policies(boost::extents[batch_size]
                               [game.ClassMethods().GetMaxNumberOfMoves()]),
      m_evaluator(evaluator),
      m_noise_epsilon(0.),
      m_noise_alpha(1.),
      m_thread_pool(thread_pool),
      m_selection_tasks(boost::extents[batch_size]),
      m_expansion_and_backpropagation_tasks(boost::extents[batch_size]) {
  Initialise();
  PerformSearch();
}

Search::Search(const oaz::games::Game& game, const Selector& selector,
               std::shared_ptr<oaz::evaluator::Evaluator> evaluator,
               std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
               size_t batch_size, size_t n_iterations, float noise_epsilon,
               float noise_alpha)
    : m_root(std::make_shared<oaz::mcts::SearchNode>()),
      m_policy_size(game.ClassMethods().GetMaxNumberOfMoves()),
      m_game(std::move(game.Clone())),
      m_selector(std::move(selector.Clone())),
      m_batch_size(batch_size),
      m_n_iterations(n_iterations),
      m_n_selections(0),
      m_n_completions(0),
      m_n_evaluation_requests(0),
      m_n_active_tasks(0),
      m_nodes(batch_size),
      m_paused_nodes(batch_size),
      m_games(batch_size),
      m_values(boost::extents[batch_size]),
      m_policies(boost::extents[batch_size]
                               [game.ClassMethods().GetMaxNumberOfMoves()]),
      m_evaluator(evaluator),
      m_noise_epsilon(noise_epsilon),
      m_noise_alpha(noise_alpha),
      m_thread_pool(thread_pool),
      m_selection_tasks(boost::extents[batch_size]),
      m_expansion_and_backpropagation_tasks(boost::extents[batch_size]) {
  Initialise();
  PerformSearch();
}

void Search::BackpropagateNode(oaz::mcts::SearchNode* node,
                               float value) {  // No need to pass game any more
  /* spdlog::debug("Search {:p} node {:p} backpropagateNode", (void*)this,
   * (void*)node); */

  // value is wrt current player
  while (!node->IsRoot()) {
    value = 1. - value;
    node->Lock();
    node->AddValue(value);
    node->Unlock();
    node = node->GetParent();
  }

  /* spdlog::debug("Search {:p} node {:p} backpropagateNode returning",
   * (void*)this, (void*)node); */
}

size_t Search::GetBatchSize() const { return m_batch_size; }

void Search::HandleCreatedTask() { m_n_active_tasks++; }

void Search::HandleFinishedTask() {
  m_n_active_tasks--;
  if (Done()) m_condition.notify_one();
}

size_t Search::GetNActiveTasks() const { return m_n_active_tasks; }

oaz::mcts::SearchNode* Search::GetNode(size_t index) { return m_nodes[index]; }

void Search::PerformSearch() {
  /* spdlog::info("Search {:p} searching", (void*)this); */
  for (size_t i = 0; i != GetBatchSize(); ++i) MaybeSelect(i);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_condition.wait(lock, [this] { return Done(); });
  /* spdlog::info("Search {:p} returning", (void*)this); */
}

void Search::SetNode(size_t index, oaz::mcts::SearchNode* node) {
  m_nodes[index] = node;
}

std::shared_ptr<oaz::mcts::SearchNode> Search::GetTreeRoot() { return m_root; }

/*  */
/* typename Search<Game, Selector>::Move Search<Game, Selector>::getBestMove() {
 */
/* 	Move best_move = 0; */
/* 	size_t best_n_visits = 0; */
/* 	for(size_t i=0; i!= m_root.getNChildren(); ++i) { */
/* 		Node* node = m_root.getChild(i); */
/* 		size_t node_n_visits = node->getNVisits(); */
/* 		if(node_n_visits > best_n_visits) { */
/* 			best_n_visits = node_n_visits; */
/* 			best_move = node->getMove(); */
/* 		} */
/* 	} */
/* 	return best_move; */
/* } */

/* void Search<Game, Selector>::getVisitCounts(Policy& move_counts) { */
/* 	for(size_t i=0; i!= m_root.getNChildren(); ++i) { */
/* 		Node* node = m_root.getChild(i); */
/* 		move_counts[node->getMove()] = node->getNVisits(); */
/* 	} */
/* } */
Search::~Search() {}
