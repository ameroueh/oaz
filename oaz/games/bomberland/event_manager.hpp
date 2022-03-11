#ifndef OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_
#define OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_

#include <queue>

#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/board.hpp"

namespace oaz::games::bomberland {

using Event = std::pair<size_t, Coordinates>;

class EventComparator {
  public:
    bool operator()(const Event& lhs, const Event& rhs) {
      return lhs.first < rhs.first;
    }
};

class EventManager {
  public:
    EventManager();
    void AddEvent(Coordinates position, size_t time);
    void AddEventFromTileAtPosition(Coordinates position, Board& board);
    void ClearEvents(Board& board, size_t tick);
  private:
    void ProcessEventAtCoordinates(Coordinates position, Board& board, size_t tick);       std::priority_queue<Event, std::vector<Event>, EventComparator> m_event_queue;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_EVENT_MANAGER_HPP_
