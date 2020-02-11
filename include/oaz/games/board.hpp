#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <array>
#include <memory>

#include "stdint.h"

#include "boost/multi_array.hpp"

/* #include <iostream> */

template <typename T, uint32_t width, uint32_t height, uint32_t n_players>
class ArrayBoard3D {
	public:
		using Array = boost::multi_array<T, 3>;
		using ArrayPtr = std::shared_ptr<Array>;
		
		T& operator()(uint32_t i, uint32_t j, uint32_t k) {
			return (*m_board)[i][j][k];
		}
		
		T operator() (uint32_t i, uint32_t j, uint32_t k) const {
			return (*m_board)[i][j][k];
		}
		
		ArrayBoard3D(): m_board(new Array(boost::extents[width][height][n_players])) {
		}

		static const uint32_t NumDimensions = 3;

		std::array<uint32_t, 3> dimensions() const {
			return {width, height, n_players};
		}

		bool operator==(const ArrayBoard3D<T, width, height, n_players>& rhs) const {
			return *m_board == *(rhs.m_board);
		}
	private:
		ArrayPtr m_board;

};
#endif
