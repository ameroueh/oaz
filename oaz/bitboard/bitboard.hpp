#ifndef __BITBOARD_HPP__
#define __BITBOARD_HPP__

#include "stdint.h"
#include <bitset>
#include <utility>
#include <iostream>

#include "oaz/bitboard/helpers.hpp"
#include "oaz/array/array.hpp"


namespace oaz::bitboard {
	
	template <size_t NROWS, size_t NCOLS>
	class BitBoard {
		public:
			constexpr BitBoard(): m_board(0ull) {}
			constexpr BitBoard(
				const std::initializer_list<std::pair<size_t, size_t>>& token_positions
			): m_board(0ull) {
				for(auto position: token_positions)
					Set(position.first, position.second);
			}
			constexpr BitBoard(const BitBoard& board): m_board(board.m_board) {}

			constexpr size_t Get(size_t i, size_t j) const {
				return (m_board >> (i * NCOLS + j)) & 1ull;
			}
			constexpr void Set(size_t i, size_t j) {
				m_board |= (1ull << (i * NCOLS + j));
				m_board &= BOARD_MASK;
			}
			constexpr void Unset(size_t i, size_t j) {
				m_board &= (~1ull << (i * NCOLS + j));
			}
			constexpr size_t Sum() const {
				return std::bitset<64>(m_board).count();
			}
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
			
			constexpr void NegativeRowShift(size_t i) {
				m_board >>= (i * NCOLS);
			}
			
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

			constexpr size_t LexicographicComponentLength(
				const BitBoard& mask_board, size_t row, size_t column
			) const {
				size_t bit_position = row * NCOLS + column;
				uint64_t masked_board = m_board & mask_board.m_board;
				uint64_t filled_masked_board = m_board | ~mask_board.m_board;
				uint64_t lhs_masked_board = masked_board >> bit_position;
				uint64_t rhs_masked_board = masked_board << (64 - bit_position);
				uint64_t lhs_filled_masked_board = filled_masked_board >> bit_position;
				uint64_t rhs_filled_masked_board = filled_masked_board << (64 - bit_position);
				uint64_t lhs_lead_mask = (1ll << tocount_ll(lhs_filled_masked_board)) - 1;

				uint64_t locount_rhs_filled_masked_board = locount_ll(rhs_filled_masked_board);
				uint64_t rhs_lead_mask = locount_rhs_filled_masked_board == 0 ? 0ull
					: ~((1ull << (64 - locount_rhs_filled_masked_board)) - 1);

				return popcount_ll(lhs_masked_board & lhs_lead_mask) 
					+ popcount_ll(rhs_masked_board & rhs_lead_mask);
			}

			void WriteToArray(oaz::array::Array<NROWS, NCOLS>& array) const {
				for(size_t i=0; i!=NROWS; ++i)
					for(size_t j=0; j!=NCOLS; ++j)
						array[i][j] = Get(i, j) ? 1. : 0.;
			}

			uint64_t GetBits() const {
				return m_board;
			}

		private:
			BitBoard(uint64_t board): m_board(board) {}

			uint64_t ColumnSegmentMask(size_t j) const {
				return (COLUMN_MASK << j) - COLUMN_MASK;
			}

			static constexpr uint64_t ROW_MASK = (NCOLS == 64) ? ~0ull : ((1ull << NCOLS) - 1);
			static constexpr uint64_t COLUMN_MASK = (NROWS == 64) ? ~0ull : ColumnMask<NROWS, NCOLS>::value;
			static constexpr uint64_t BOARD_MASK = (NCOLS * NROWS == 64) ? ~0ull : ((1ull << NCOLS*NROWS) - 1);

			uint64_t m_board;
	};

}
#endif