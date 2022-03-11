#ifndef OAZ_GAMES_BOMBERLAND_FIRE_ADDER_HPP_
#define OAZ_GAMES_BOMBERLAND_FIRE_ADDER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/bomb_detonator.hpp"

namespace oaz::games::bomberland {

class FireAdder {
  public:
    FireAdder():
      m_cursor0(Coordinates(0,0)),
      m_cursor1(Coordinates(14,14)),
      m_vector0(Coordinates(1,0)),
      m_vector1(Coordinates(-1,0)) {}
      
      void operator()(Board& board, EventManager& event_manager, size_t tick) {
        AddFire(board, m_cursor0, m_vector0, event_manager, tick);
	AddFire(board, m_cursor1, m_vector1, event_manager, tick);
      }
  private:
      Coordinates m_cursor0;
      Coordinates m_cursor1;
      Coordinates m_vector0;
      Coordinates m_vector1;
      
      void AddFire(
	Board& board,
	Coordinates& cursor,
	Coordinates& vector,
	EventManager& event_manager,
	size_t tick) {
        Tile& tile = board.GetTile(cursor);
	if (tile.HasFire()) { return; }
	if (tile.HasPlacedBomb()) {
	  BombDetonator(cursor, board, event_manager, tick);
	}
	tile = Tile::CreateTileWithFire();
	Coordinates new_cursor = cursor + vector;
	if (!board.IsWithinBounds(new_cursor) || board.GetTile(new_cursor).HasFire()) {
          vector = GetNextVector(vector);
	  cursor += vector;
        } else {
          cursor = new_cursor;
	}
      }
      
      Coordinates GetNextVector(const Coordinates& vector) {
        if (vector == Coordinates(1, 0)) { return Coordinates(0, 1); }
	if (vector == Coordinates(0, 1)) { return Coordinates(-1, 0); }
	if (vector == Coordinates(-1, 0)) { return Coordinates(0, -1); }
	if (vector == Coordinates(0, -1)) { return Coordinates(1, 0); }
      }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_FIRE_ADDER_HPP_
