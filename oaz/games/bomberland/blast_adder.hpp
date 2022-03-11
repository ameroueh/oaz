#ifndef OAZ_GAMES_BOMBERLAND_BLAST_ADDER_HPP_
#define OAZ_GAMES_BOMBERLAND_BLAST_ADDER_HPP_

#include <cstddef>
#include <queue>
#include <iostream>

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/detonation_order.hpp"
#include "oaz/games/bomberland/event_manager.hpp"

namespace oaz::games::bomberland {

class BlastAdder {
  public:
    BlastAdder(
      DetonationOrder order,
      Board& board,
      std::queue<DetonationOrder>& detonation_orders,
      EventManager& event_manager,
      std::size_t tick
    ):
      m_order(order),
      m_board(board),
      m_detonation_orders(detonation_orders),
      m_event_manager(event_manager),
      m_tick(tick)
      {
	AddBlast();
      }
  private:
    void AddBlast() {
      ProcessTile(m_board.GetTile(m_order.first), m_order.first);
      AddBlastAlongLine(Coordinates(1, 0));
      AddBlastAlongLine(Coordinates(-1, 0));
      AddBlastAlongLine(Coordinates(0, 1));
      AddBlastAlongLine(Coordinates(0, -1));
    }
    bool ProcessTile(Tile& tile, Coordinates position) {
      if (tile.HasFire()) {
        return false;
      } else if (!tile.IsWalkable()) {
	tile.DecreaseHealth();
	if (tile.GetHP() == 0) {
          tile = Tile::CreateEmptyTile();
	}
	return false;
      } else if (tile.HasPlacedBomb()) {
	m_detonation_orders.push(
          DetonationOrder(
	    position, tile.GetBlastRadius()
	  )	  
	);
        tile = Tile::CreateTileWithBlast(m_tick);
	m_event_manager.AddEventFromTileAtPosition(position, m_board);
	return true;
      } else {
        tile = Tile::CreateTileWithBlast(m_tick);
	m_event_manager.AddEventFromTileAtPosition(position, m_board);
        return true;
      }
    }
    void AddBlastAlongLine(Coordinates vector) {
      Coordinates current_position = m_order.first + vector;
      for (std::size_t i=0; i!=m_order.second; ++i) {
	if (!m_board.IsWithinBounds(current_position)) {
	  break;
	}
	bool no_obstacle = ProcessTile(m_board.GetTile(current_position), current_position);
	if(!no_obstacle) {
	  break;
	}
	current_position += vector;
      }
    }

    DetonationOrder m_order;
    Board& m_board;
    EventManager& m_event_manager;
    std::queue<DetonationOrder>& m_detonation_orders;
    std::size_t m_tick;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_BLAST_ADDER_HPP_
