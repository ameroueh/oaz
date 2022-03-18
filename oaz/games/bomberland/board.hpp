#ifndef OAZ_GAMES_BOMBERLAND_BOARD_HPP_
#define OAZ_GAMES_BOMBERLAND_BOARD_HPP_

#include "boost/multi_array.hpp"

#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/coordinates.hpp"

namespace oaz::games::bomberland {

class Board {
  public:
    Board():
      m_n_rows(15),
      m_n_columns(15),
      m_board(boost::extents[GetNRows()][GetNColumns()]) {
      std::fill_n(m_board.data(), m_board.num_elements(), Tile::CreateEmptyTile());
    }
    Board(size_t n_rows, size_t n_columns):
	m_n_rows(n_rows), 
	m_n_columns(n_columns),
	m_board(boost::extents[GetNRows()][GetNColumns()]) {
      std::fill_n(m_board.data(), m_board.num_elements(), Tile::CreateEmptyTile());
    }
    Board(size_t n_rows, size_t n_columns, const Tile&& tile):
      m_n_rows(n_rows),
      m_n_columns(n_columns),
      m_board(boost::extents[GetNRows()][GetNColumns()]) {
      std::fill_n(m_board.data(), m_board.num_elements(), tile);
    }
    bool IsWithinBounds(Coordinates position) const {
      int first = position.first();
      int second = position.second();
      return 0 <= first && first < GetNRows() && 0 <= second && second < GetNColumns(); 
    }
    size_t GetNRows() const {
      return m_n_rows;
    }
    size_t GetNColumns() const {
      return m_n_columns;
    }
    Tile& GetTile(Coordinates coords) {
      return m_board[coords.first()][coords.second()];
    }
    const Tile& GetTile(Coordinates coords) const {
      return m_board[coords.first()][coords.second()];
    }
  private:
    size_t m_n_rows;
    size_t m_n_columns;
    boost::multi_array<Tile, 2> m_board;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_BOARD_HPP_
