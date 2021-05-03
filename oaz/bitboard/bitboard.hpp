#ifndef OAZ_BITBOARD_BITDOARB_HPP_
#define OAZ_BITBOARD_BITDOARB_HPP_

#include <stdint.h>

#include <bitset>
#include <iostream>
#include <utility>

#include "oaz/array/array.hpp"
#include "oaz/bitboard/helpers.hpp"

namespace oaz::bitboard {

template <size_t NROWS, size_t NCOLS>
class BitBoard {
 public:
  static constexpr uint64_t N_BITS = 64;
  constexpr BitBoard() : m_board(0ULL) {}
  constexpr BitBoard(
      const std::initializer_list<std::pair<size_t, size_t>>& token_positions)
      : m_board(0ULL) {
    for (auto position : token_positions) {Set(position.first, position.second);}
  }
  constexpr size_t Get(size_t i, size_t j) const {
    return (m_board >> (i * NCOLS + j)) & 1ULL;
  } constexpr void Set(size_t i, size_t j) { m_board |= (1ULL << (i * NCOLS + j));
    m_board &= BOARD_MASK;
  }
  constexpr void Unset(size_t i, size_t j) {
    m_board &= (~1ULL << (i * NCOLS + j));
  }
  constexpr size_t Sum() const { return std::bitset<N_BITS>(m_board).count(); }
  constexpr size_t RowSum(size_t i) const {
    return popcount_ll((m_board >> (i * NCOLS)) & ROW_MASK);
  }
  constexpr size_t ColumnSum(size_t j) const {
    return popcount_ll((m_board >> j) & COLUMN_MASK);
  }
  constexpr BitBoard operator|(const BitBoard& board) const {
    return BitBoard(m_board | board.m_board);
  }

  constexpr BitBoard operator&(const BitBoard& board) const {
    return BitBoard(m_board & board.m_board);
  }

  constexpr void PositiveRowShift(size_t i) {
    m_board <<= (i * NCOLS);
    m_board &= BOARD_MASK;
  }

  constexpr void NegativeRowShift(size_t i) { m_board >>= (i * NCOLS); }

  constexpr void PositiveColumnShift(size_t j) {
    m_board &= ColumnSegmentMask(NCOLS - j);
    m_board <<= j;
  }

  constexpr void NegativeColumnShift(size_t j) {
    m_board &= ~ColumnSegmentMask(j);
    m_board >>= j;
  }

  constexpr bool operator==(const BitBoard& rhs) const {
    return m_board == rhs.m_board;
  }

  constexpr bool IsContainedIn(const BitBoard& board) const {
    return ((*this) & board) == (*this);
  }

  constexpr size_t LexicographicComponentLength(const BitBoard& mask_board,
                                                size_t row,
                                                size_t column) const {
    size_t bit_position = row * NCOLS + column;
    uint64_t masked_board = m_board & mask_board.m_board;
    uint64_t filled_masked_board = m_board | ~mask_board.m_board;
    uint64_t lhs_masked_board = masked_board >> bit_position;
    uint64_t rhs_masked_board = masked_board << (N_BITS - bit_position);
    uint64_t lhs_filled_masked_board = filled_masked_board >> bit_position;
    uint64_t rhs_filled_masked_board = filled_masked_board
                                       << (N_BITS - bit_position);
    uint64_t lhs_lead_mask = (1LL << tocount_ll(lhs_filled_masked_board)) - 1;

    uint64_t locount_rhs_filled_masked_board =
        locount_ll(rhs_filled_masked_board);
    uint64_t rhs_lead_mask =
        locount_rhs_filled_masked_board == 0
            ? 0ULL
            : ~((1ULL << (N_BITS - locount_rhs_filled_masked_board)) - 1);

    return popcount_ll(lhs_masked_board & lhs_lead_mask) +
           popcount_ll(rhs_masked_board & rhs_lead_mask);
  }

  void WriteToArray(oaz::array::Array<NROWS, NCOLS>& array) const {
    for (size_t i = 0; i != NROWS; ++i) {
      for (size_t j = 0; j != NCOLS; ++j) {array[i][j] = Get(i, j) ? 1. : 0.;}
    }
  }

  uint64_t GetBits() const { return m_board; }

 private:
  explicit BitBoard(uint64_t board) : m_board(board) {}

  uint64_t ColumnSegmentMask(size_t j) const {
    return (COLUMN_MASK << j) - COLUMN_MASK;
  }

  static constexpr uint64_t ROW_MASK =
      (NCOLS == N_BITS) ? ~0ULL : ((1ULL << NCOLS) - 1);
  static constexpr uint64_t COLUMN_MASK =
      (NROWS == N_BITS) ? ~0ULL : ColumnMask<NROWS, NCOLS>::value;
  static constexpr uint64_t BOARD_MASK =
      (NCOLS * NROWS == N_BITS) ? ~0ULL : ((1ULL << NCOLS * NROWS) - 1);

  uint64_t m_board;
};

}  // namespace oaz::bitboard
#endif  // OAZ_BITBOARD_BITBOARD_HPP_
