#ifndef OAZ_GAMES_BOMBERLAND_BOMB_LIST_CLEANER_HPP_
#define OAZ_GAMES_BOMBERLAND_BOMB_LIST_CLEANER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/bomb_detonator.hpp"

namespace oaz::games::bomberland {

class BombListCleaner {
  public:
    BombListCleaner() {}
      
      void operator()(size_t owner, std::vector<Coordinates>& bombs, const Board& board) {
	size_t end = bombs.size();
	size_t i=0;
	while (i < end) {
	  const Tile& tile = board.GetTile(bombs[i]);
          if (!tile.HasPlacedBomb() || tile.GetOwner() != owner) {
	    bombs[i] = bombs[end-1];
	    --end;
	  } else {
	    ++i;
	  }
	}
	bombs.resize(end);
      }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_BOMB_LIST_CLEANER_HPP_
