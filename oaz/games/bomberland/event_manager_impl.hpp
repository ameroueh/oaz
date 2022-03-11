#ifndef OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_IMPL_HPP_
#define OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_IMPL_HPP_

#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/bomb_detonator.hpp"

oaz::games::bomberland::EventManager::EventManager() {}

void oaz::games::bomberland::EventManager::AddEvent(Coordinates position, size_t time) {
  m_event_queue.push(Event(time, position));
}

void oaz::games::bomberland::EventManager::AddEventFromTileAtPosition(Coordinates position, Board& board) {
  size_t time = board.GetTile(position).GetExpiryTime();
  AddEvent(position, time);
}

void oaz::games::bomberland::EventManager::ClearEvents(Board& board, size_t tick) {
  if(m_event_queue.empty()) { return; }
  Event tcoords = m_event_queue.top();
  size_t time = tcoords.first;
  while(time == tick && !m_event_queue.empty()) {
	  Coordinates position = tcoords.second;
	  ProcessEventAtCoordinates(position, board, tick);
	  m_event_queue.pop();
  }
}

void oaz::games::bomberland::EventManager::ProcessEventAtCoordinates(Coordinates position, Board& board, size_t tick) {
      Tile& tile = board.GetTile(position);
      if (!tile.IsWalkable() || tile.GetExpiryTime() != tick) {
        return;
      }
      if (tile.HasBlast()) {
        tile = Tile::CreateEmptyTile();
      } else if (tile.HasPlacedBomb()) {
        BombDetonator(position, board, *this, tick); 
      } else if (tile.HasSpawnedBomb()) {
        tile = Tile::CreateEmptyTile();
      } else if (tile.HasSpawnedPowerup()) {
	tile = Tile::CreateEmptyTile();
      }
}
#endif // OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_IMPL_HPP_
