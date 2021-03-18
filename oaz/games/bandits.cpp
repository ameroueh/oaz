#include "oaz/games/bandits.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

oaz::games::Bandits::Bandits() {}

void oaz::games::Bandits::Reset() { *this = Bandits(); }

void oaz::games::Bandits::PlayFromString(std::string moves) {
  for (char& c : moves) {
    PlayMove(c - '0');
  }
}

void oaz::games::Bandits::PlayMove(size_t move) { m_board.set(move); }

void oaz::games::Bandits::GetAvailableMoves(
    std::vector<size_t>& available_moves) const {
  available_moves.clear();

  for (size_t i = 0; i != 10; ++i)
    if (!m_board.test(i)) available_moves.push_back(i);
}

bool oaz::games::Bandits::IsFinished() const { return GetCurrentPlayer() == 1; }

size_t oaz::games::Bandits::GetCurrentPlayer() const {
  return m_board.any() ? 1 : 0;
}

float oaz::games::Bandits::GetScore() const {
  return (m_board & WINNING_BITS).any() ? 1. : -1.;
}

std::unique_ptr<oaz::games::Game> oaz::games::Bandits::Clone() const {
  return std::make_unique<Bandits>(*this);
}

bool oaz::games::Bandits::operator==(const Bandits& rhs) const {
  return m_board == rhs.m_board;
}

void oaz::games::Bandits::WriteStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, 1> tensor(destination, boost::extents[10]);
  for (size_t i = 0; i != 10; ++i) tensor[i] = m_board.test(i) ? 1. : 0.;
}

void oaz::games::Bandits::WriteCanonicalStateToTensorMemory(
    float* destination) const {
  WriteStateToTensorMemory(destination);
}

void oaz::games::Bandits::InitialiseFromState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 1> data(input_board, boost::extents[10]);

  for (size_t i = 0; i != 10; ++i) {
    if (data[i] == 1.0f) m_board.set(i);
  }
}

void oaz::games::Bandits::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 1> data(input_board, boost::extents[10]);

  for (size_t i = 0; i != 10; ++i) {
    if (data[i] == 1.0f) m_board.set(i);
  }
}

uint64_t oaz::games::Bandits::GetState() const { return m_board.to_ulong(); }
