#include "oaz/games/bandits.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace oaz::games;

Bandits::Bandits() {}

void Bandits::Reset() { *this = Bandits(); }

void Bandits::PlayFromString(std::string moves) {
  for (char& c : moves) {
    PlayMove(c - '0');
  }
}

void Bandits::PlayMove(size_t move) { m_board.set(move); }

void Bandits::GetAvailableMoves(std::vector<size_t>& available_moves) const {
  available_moves.clear();

  for (size_t i = 0; i != 10; ++i)
    if (!m_board.test(i)) available_moves.push_back(i);
}

bool Bandits::IsFinished() const { return GetCurrentPlayer() == 1; }

size_t Bandits::GetCurrentPlayer() const { return m_board.any() ? 1 : 0; }

float Bandits::GetScore() const {
  return (m_board & WINNING_BITS).any() ? 1. : -1.;
}

std::unique_ptr<Game> Bandits::Clone() const {
  return std::make_unique<Bandits>(*this);
}

bool Bandits::operator==(const Bandits& rhs) const {
  return m_board == rhs.m_board;
}

void Bandits::WriteStateToTensorMemory(float* destination) const {
  boost::multi_array_ref<float, 1> tensor(destination, boost::extents[10]);
  for (size_t i = 0; i != 10; ++i) tensor[i] = m_board.test(i) ? 1. : 0.;
}

void Bandits::WriteCanonicalStateToTensorMemory(float* destination) const {
  WriteStateToTensorMemory(destination);
}

void Bandits::InitialiseFromState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 1> data(input_board, boost::extents[10]);

  for (size_t i = 0; i != 10; ++i) {
    if (data[i] == 1.0f) m_board.set(i);
  }
  // TODO Check victory
}

void Bandits::InitialiseFromCanonicalState(float* input_board) {
  Reset();
  boost::multi_array_ref<float, 1> data(input_board, boost::extents[10]);

  for (size_t i = 0; i != 10; ++i) {
    if (data[i] == 1.0f) m_board.set(i);
  }
  // TODO Check victory
}

uint64_t Bandits::GetState() const { return m_board.to_ulong(); }