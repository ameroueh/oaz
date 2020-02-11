#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "nlohmann/json.hpp"

template <class Board>
void loadBoardFromJson(const nlohmann::json& data, Board& board) {
	for(int i=0; i!=board.dimensions()[0]; ++i)
		for(int j=0; j!=board.dimensions()[1]; ++j) 
			for(int k=0; k!=board.dimensions()[2]; ++k) 
				board(i, j, k) = data[i][j][k];
}
#endif
