#ifndef OAZ_GAMES_BOMBERLAND_BOMB_DETONATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_BOMB_DETONATOR_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/blast_adder.hpp"
#include "oaz/games/bomberland/detonation_order.hpp"

namespace oaz::games::bomberland {

class BombDetonator {
  public:
    BombDetonator(Coordinates position, Board& board, size_t tick):
    m_position(position), m_board(board), m_tick(tick) {
      Detonate();
    }
  private:
    void Detonate() {
      Tile& tile = m_board.GetTile(m_position);
      if (!tile.HasPlacedBomb()) {
        return;
      }
      size_t blast_radius = tile.GetBlastRadius();
      std::queue<DetonationOrder> detonation_orders;
      detonation_orders.push(
        DetonationOrder(m_position, blast_radius)
      );
      while (!detonation_orders.empty()) {
	DetonationOrder order = detonation_orders.front();
	detonation_orders.pop();
	BlastAdder(
	    order,
	    m_board,
	    detonation_orders,
	    m_tick
	);
      }
    }
    Coordinates m_position;
    Board& m_board;
    size_t m_tick;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_BOMB_DETONATOR_HPP_
