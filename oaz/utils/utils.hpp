#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <vector>
#include "nlohmann/json.hpp"

template <class Game>
void loadBoardFromJson(const nlohmann::json& data, typename Game::Board& board) {
	auto dimensions = Game::Board::Dimensions();
	for(int i=0; i!=dimensions[0]; ++i)
		for(int j=0; j!=dimensions[1]; ++j) 
			for(int k=0; k!=dimensions[2]; ++k) 
				board[i][j][k] = data[i][j][k];
}

template <class ArrayRef, int N>
struct RecursiveLoader {
	static void loadMultiArrayFromJson(const nlohmann::json& data, ArrayRef array) {
		for(int i=0; i!=array.shape()[0]; ++i)
			RecursiveLoader< typename ArrayRef::template subarray<N-1>::type, N-1 >::loadMultiArrayFromJson(
				data[i], 
				array[i]
			);
	}
};

template <class ArrayRef>
struct RecursiveLoader<ArrayRef, 1> {
	static void loadMultiArrayFromJson(const nlohmann::json& data, ArrayRef array) {
		for(int i=0; i!=array.shape()[0]; ++i) {
			array[i] = data[i];
		}
	}
};

template <class Array>
void loadMultiArrayFromJson(const nlohmann::json& data, Array& array) {
	RecursiveLoader<Array, Array::dimensionality>::loadMultiArrayFromJson(data, array);
}

template <class Array>
void loadArrayFromJson(const nlohmann::json& data, Array& array) {
	for(size_t i=0; i!=array.size(); ++i)
		array[i] = data[i];
}

#endif
