#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <array>
#include <memory>

#include "stdint.h"

#include "boost/multi_array.hpp"

template <typename T>
class MultiArrayBoard {
	public:
		using Array = boost::multi_array<T, 3>;
		using ArrayPtr = std::unique_ptr<Array>;
		
		T& operator()(size_t i, size_t j, size_t k) {
			return (*m_board)[i][j][k];
		}
		
		T operator() (size_t i, size_t j, size_t k) const {
			return (*m_board)[i][j][k];
		}
		
		ArrayBoard3D(): m_board(new Array(boost::extents[width][height][n_players])) {
		}

		static const size_t NumDimensions = 3;

		static const std::array<size_t, 3> dimensions() {
			return {width, height, n_players};
		}

		bool operator==(const ArrayBoard3D<T, width, height, n_players>& rhs) const {
			return *m_board == *(rhs.m_board);
		}

		ArrayBoard3D(const ArrayBoard3D& board):m_board(new Array(boost::extents[width][height][n_players])) {
			*m_board = *(board.m_board);
			
		}
	private:
		template <size_t
		template accesArrayElement(
		ArrayPtr m_board;

};
#endif
