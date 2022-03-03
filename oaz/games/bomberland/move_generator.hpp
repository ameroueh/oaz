#ifndef OAZ_GAMES_BOMBERLAND_MOVE_GENERATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_MOVE_GENERATOR_HPP_

namespace oaz::games::bomberland {

class MoveGenerator {
  public:
    bool IsWithinBounds(Coordinates position) const {
      int first = position.first();
      int second = position.second();
      return 0 <= first && first < GetNRows() && 0 <= second && second < GetNColumns(); 
    }
    size_t GetNRows() const {
      return N_ROWS;
    }
    size_t GetNColumns() const {
      return N_COLUMNS;
    }
    Tile& GetTile(Coordinates coords) {
      return m_board[coords.first()][coords.second()];
    }
    const Tile& GetTile(Coordinates coords) const {
      return m_board[coords.first()][coords.second()];
    }
    MoveGenerator(): m_board(boost::extents[GetNRows()][GetNColumns()]) {
      std::fill_n(m_board.data(), m_board.num_elements(), Tile::CreateEmptyTile());
    }
  private:
    boost::multi_array<Tile, 2> m_board;
    static constexpr size_t N_ROWS = 15;
    static constexpr size_t N_COLUMNS = 15;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_MOVE_GENERATOR_HPP_
