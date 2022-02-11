#include "oaz/mcts/search.hpp"

#include <stdint.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "boost/multi_array.hpp"
#include "oaz/mcts/search_node.hpp"
#include "oaz/mcts/selection.hpp"

oaz::mcts::PlayerSearchProperties::PlayerSearchProperties(
  std::shared_ptr<oaz::evaluator::Evaluator> evaluator,
  std::shared_ptr<oaz::mcts::Selector> selector
): m_evaluator(evaluator), m_selector(selector) {}

std::shared_ptr<oaz::evaluator::Evaluator>& oaz::mcts::PlayerSearchProperties::GetEvaluator() {
  return m_evaluator;
}

std::shared_ptr<oaz::mcts::Selector>& oaz::mcts::PlayerSearchProperties::GetSelector() {
  return m_selector; 
}

oaz::mcts::Search::SelectionTask::SelectionTask(oaz::mcts::Search* search,
                                                size_t index)
    : m_index(index), m_search(search) {
  m_search->HandleCreatedTask();
}

oaz::mcts::Search::SelectionTask::SelectionTask()
    : m_index(0), m_search(nullptr) {}

void oaz::mcts::Search::SelectionTask::operator()() {
  m_search->SelectNode(m_index);
  m_search->HandleFinishedTask();
}

oaz::mcts::Search::ExpansionAndBackpropagationTask::
    ExpansionAndBackpropagationTask(oaz::mcts::Search* search, size_t index)
    : m_index(index), m_search(search) {
  m_search->HandleCreatedTask();
}

oaz::mcts::Search::ExpansionAndBackpropagationTask::
    ExpansionAndBackpropagationTask()
    : m_index(0), m_search(nullptr) {}

void oaz::mcts::Search::ExpansionAndBackpropagationTask::operator()() {
  m_search->ExpandAndBackpropagateNode(m_index);
  m_search->HandleFinishedTask();
}

void oaz::mcts::Search::SelectNode(size_t index) {
  oaz::mcts::SearchNode* node = GetNode(index);
  oaz::games::Game* game = GetGame(index);

  size_t current_player = game->GetCurrentPlayer();

  while (true) {
    node->Lock();
    if (!node->IsLeaf()) {
      node->IncrementNVisits();
      node->Unlock();
      size_t child_index =
	(*(m_player_search_properties[current_player].GetSelector()))(node);
      node = node->GetChild(child_index);
      game->PlayMove(node->GetMove());
      SetNode(index, node);

    } else if (node->IsBlockedForEvaluation()) {
      Pause(index);
      node->Unlock();
      break;
    } else {
      node->IncrementNVisits();
      if (!game->IsFinished()) {
        node->BlockForEvaluation();
      }
      node->Unlock();
      m_n_evaluation_requests++;

      m_expansion_and_backpropagation_tasks[index] =
          ExpansionAndBackpropagationTask(this, index);

      m_player_search_properties[current_player].GetEvaluator()->RequestEvaluation(
          game, GetEvaluation(index), &m_expansion_and_backpropagation_tasks[index]);
      break;
    }
  }
}

void oaz::mcts::Search::Pause(size_t index) {
  m_paused_nodes[index] = GetNode(index);
}

void oaz::mcts::Search::AddDirichletNoise(
    boost::multi_array<float, 1>& policy) {
  size_t policy_size = policy.num_elements();
  if (m_noise_epsilon > EPS_THRESHOLD) {
    boost::multi_array<float, 1> noise(boost::extents[policy_size]);
    float total_noise = 0.;
    for (size_t i = 0; i != policy_size; ++i) {
      std::gamma_distribution<float> distribution(m_noise_alpha, 1.);
      noise[i] = distribution(m_generator);
      total_noise += noise[i];
    }
    for (size_t i = 0; i != policy_size; ++i) {
      policy[i] = (1 - m_noise_epsilon) * policy[i] +
                  m_noise_epsilon * noise[i] / total_noise;
    }
  }
}

void oaz::mcts::Search::ExpandNode(oaz::mcts::SearchNode* node,
                                   oaz::games::Game* game,
                                   oaz::evaluator::Evaluation* evaluation) {
  /* if (node->IsRoot()) { */
  /*   AddDirichletNoise(policy); // PROBLEM ! */
  /* } */

  std::vector<size_t> available_moves;
  game->GetAvailableMoves(&available_moves);
  size_t player = game->GetCurrentPlayer();

  for (auto move : available_moves) {
    float prior = evaluation->GetPolicy(move);
    node->AddChild(move, player, prior);
  }
}

void oaz::mcts::Search::Unpause(oaz::mcts::SearchNode* node) {
  for (size_t index = 0; index != GetBatchSize(); ++index) {
    if (m_paused_nodes[index] == node) {
      m_selection_tasks[index] = SelectionTask(this, index);
      m_thread_pool->enqueue(&m_selection_tasks[index]);
      m_paused_nodes[index] = nullptr;
    }
  }
}

size_t oaz::mcts::Search::GetNSelections() const { return m_n_selections; }

size_t oaz::mcts::Search::GetNIterations() const { return m_n_iterations; }

void oaz::mcts::Search::MaybeSelect(size_t index) {
  m_selection_lock.Lock();
  if (GetNSelections() < GetNIterations()) {
    ++m_n_selections;
    m_selection_lock.Unlock();
    m_selection_tasks[index] = SelectionTask(this, index);
    m_thread_pool->enqueue(&m_selection_tasks[index]);
  } else {
    m_selection_lock.Unlock();
  }
}

oaz::games::Game* oaz::mcts::Search::GetGame(size_t index) {
  return m_games[index].get();
}

std::unique_ptr<oaz::evaluator::Evaluation>* oaz::mcts::Search::GetEvaluation(size_t index) {
  return &m_evaluations[index];
}

void oaz::mcts::Search::ExpandAndBackpropagateNode(size_t index) {

  oaz::evaluator::Evaluation* evaluation = (*GetEvaluation(index)).get();
  float value = evaluation->GetValue();
  value = (value + 1.0F) / 2.0F;  // NOLINT

  oaz::mcts::SearchNode* node = GetNode(index);
  oaz::games::Game* game = GetGame(index);

  if (!game->IsFinished()) {
    node->Lock();
    ExpandNode(node, game, evaluation);
    node->UnblockForEvaluation();
    Unpause(node);
    node->Unlock();
  }

  BackpropagateNode(node, value);
  SetNode(index, m_root.get());
  IncrementNCompletions();
  ResetGame(index);
  MaybeSelect(index);
}

void oaz::mcts::Search::ResetGame(size_t index) {
  m_games[index] = std::move(m_game->Clone());
}

bool oaz::mcts::Search::Done() const {
  return (GetNCompletions() == GetNIterations()) && (GetNActiveTasks() == 0);
}

void oaz::mcts::Search::IncrementNCompletions() { ++m_n_completions; }

size_t oaz::mcts::Search::GetNCompletions() const { return m_n_completions; }

void oaz::mcts::Search::Initialise() {
  std::random_device seeder;
  m_generator.seed(seeder());

  for (size_t index = 0; index != GetBatchSize(); ++index) {
    ResetGame(index);
    m_nodes[index] = m_root.get();
    m_paused_nodes[index] = nullptr;
  }
}

oaz::mcts::Search::Search(
    const oaz::games::Game& game,
    const std::vector<PlayerSearchProperties>& player_search_properties,
    std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
    size_t batch_size, size_t n_iterations)
    : m_root(std::make_shared<oaz::mcts::SearchNode>()),
      m_game(std::move(game.Clone())),
      m_batch_size(batch_size),
      m_n_iterations(n_iterations),
      m_n_selections(0),
      m_n_completions(0),
      m_n_evaluation_requests(0),
      m_n_active_tasks(0),
      m_nodes(batch_size),
      m_paused_nodes(batch_size),
      m_games(batch_size),
      m_evaluations(boost::extents[batch_size]),
      m_noise_epsilon(0.),
      m_noise_alpha(1.),
      m_thread_pool(std::move(thread_pool)),
      m_selection_tasks(boost::extents[batch_size]),
      m_expansion_and_backpropagation_tasks(boost::extents[batch_size]),
      m_player_search_properties(player_search_properties) {
  Initialise();
  PerformSearch();
}

oaz::mcts::Search::Search(
    const oaz::games::Game& game,
    const std::vector<PlayerSearchProperties>& player_search_properties,
    std::shared_ptr<oaz::thread_pool::ThreadPool> thread_pool,
    size_t batch_size, size_t n_iterations, float noise_epsilon,
    float noise_alpha)
    : m_root(std::make_shared<oaz::mcts::SearchNode>()),
      m_game(std::move(game.Clone())),
      m_batch_size(batch_size),
      m_n_iterations(n_iterations),
      m_n_selections(0),
      m_n_completions(0),
      m_n_evaluation_requests(0),
      m_n_active_tasks(0),
      m_nodes(batch_size),
      m_paused_nodes(batch_size),
      m_games(batch_size),
      m_evaluations(boost::extents[batch_size]),
      m_noise_epsilon(noise_epsilon),
      m_noise_alpha(noise_alpha),
      m_thread_pool(std::move(thread_pool)),
      m_selection_tasks(boost::extents[batch_size]),
      m_expansion_and_backpropagation_tasks(boost::extents[batch_size]),
      m_player_search_properties(player_search_properties) {
  Initialise();
  PerformSearch();
}

void oaz::mcts::Search::BackpropagateNode(oaz::mcts::SearchNode* node,
                                          float value) {
  while (!node->IsRoot()) {
    value = 1.0F - value;
    node->Lock();
    node->AddValue(value);
    node->Unlock();
    node = node->GetParent();
  }
}

size_t oaz::mcts::Search::GetBatchSize() const { return m_batch_size; }

void oaz::mcts::Search::HandleCreatedTask() { m_n_active_tasks++; }

void oaz::mcts::Search::HandleFinishedTask() {
  m_n_active_tasks--;
  if (Done()) {
    m_condition.notify_one();
  }
}

size_t oaz::mcts::Search::GetNActiveTasks() const { return m_n_active_tasks; }

oaz::mcts::SearchNode* oaz::mcts::Search::GetNode(size_t index) {
  return m_nodes[index];
}

void oaz::mcts::Search::PerformSearch() {
  for (size_t i = 0; i != GetBatchSize(); ++i) {
    MaybeSelect(i);
  }

  std::unique_lock<std::mutex> lock(m_mutex);
  m_condition.wait(lock, [this] { return Done(); });
}

void oaz::mcts::Search::SetNode(size_t index, oaz::mcts::SearchNode* node) {
  m_nodes[index] = node;
}

std::shared_ptr<oaz::mcts::SearchNode> oaz::mcts::Search::GetTreeRoot() {
  return m_root;
}

oaz::mcts::Search::~Search() {}
