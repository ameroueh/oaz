#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <array>

#include "stdint.h"

#include "boost/multi_array.hpp"

/* #include <iostream> */

template <typename T, uint32_t width, uint32_t height, uint32_t n_players>
class ArrayBoard3D {
	public:
		T& operator()(uint32_t i, uint32_t j, uint32_t k) {
			/* std::cout << i << " " << j << " " << k << std::endl; */
			/* std::cout << m_board[0][0][0] << std::endl; */
			/* std::cout << "Passed" << std::endl; */

			return m_board[i][j][k];
		}

		ArrayBoard3D(): m_board(boost::extents[width][height][n_players]) {
			/* std::cout << "Initialising board with " << width << " " << height << " " << n_players << std::endl; */	
			/* std::cout << m_board[0][0][0] << std::endl; */
		}

		static const uint32_t NumDimensions = 3;

		std::array<uint32_t, 3> dimensions() const {
			return {width, height, n_players};
		}

		bool operator==(const ArrayBoard3D<T, width, height, n_players>& rhs) {
			return m_board == rhs.m_board;
		}
	private:
		boost::multi_array<T, 3> m_board;

};
#endif
