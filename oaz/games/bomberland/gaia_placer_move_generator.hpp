#ifndef OAZ_GAMES_BOMBERLAND_GAIA_PLACER_MOVE_GENERATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_GAIA_PLACER_MOVE_GENERATOR_HPP_

#include <vector>

namespace oaz::games::bomberland {

class GaiaPlacerMoveGenerator {
  public:
    void operator()(std::vector<size_t>& moves, const Board& board) {
	moves.clear();
	for (size_t i=0; i!=N_ROWS; ++i) {
          for (size_t j=0; j!=N_COLUMNS; ++j) {
	    Coordinates  coords = Coordinates(i, j);
	    if (board.GetTile(coords).IsEmptyTile()) {
	      moves.push_back(coords.AsUint64());
	    }
	  }
        }
    }
  private:
    static constexpr size_t N_ROWS = 15;
    static constexpr size_t N_COLUMNS = 15;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_GAIA_PLACER_MOVE_GENERATOR_HPP_
