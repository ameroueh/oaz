#ifndef OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_
#define OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_

#include <queue>

#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/bomb_detonator.hpp"

namespace oaz::games::bomberland {

using Event = std::pair<size_t, Coordinates>;

bool CompareEvents(const Event& t1, const Event& t2) {
  return t1.first < t2.first;
}

class EventManager {
  public:
    EventManager() {}
    AddEvent(Coordinates position, size_t time) {
      m_event_queue.push(Event(time, position));
    }
    void ClearEvents(size_t tick) {
      if(m_event_queue.empty()) { return; }
      Event tcoords = m_event_queue.front();
      size_t time = tcoords.first;
      while(time == m_tick) {
	Coordinates position = tcoords.second;
	ProcessEventAtCoordinates(position, board, tick);
        m_event_queue.pop();
      }
    }
  private:
    void ProcessEventAtCoordinates(Coordinates position, Board& board, size_t tick) {
      Tile& tile = board.GetTile(position);
      if (!tile.IsWalkable() || tile.GetExpiryTime() != tick) {
        return;
      }
      if (tile.HasBlast()) {
        tile = Tile::CreateEmptyTile();
      } else if (tile.HasPlacedBomb()) {
        BombDetonator(position, board, tick); 
      } else if (tile.HasSpawnedBomb()) {
        tile = Tile::CreateEmptyTile();
      } else if (tile.HasSpawnedPowerup()) {
	tile = Tile::CreateEmptyTile();
      }
    }
    std::priority_queue<Event, std::vector<Event>, CompareEvents> m_event_queue;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_
