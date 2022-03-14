#ifndef OAZ_GAMES_BOMBERLAND_GAIA_MOVE_PLAYER_HPP_
#define OAZ_GAMES_BOMBERLAND_GAIA_MOVE_PLAYER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/gaia_spawner_move_generator.hpp"

#include <iostream>

namespace oaz::games::bomberland {

class GaiaMovePlayer {
  public:
    GaiaMovePlayer(): m_spawner_move(GaiaSpawnerMove::Pass) {}
    void PlaySpawnerMove(size_t move) {
      m_spawner_move = static_cast<GaiaSpawnerMove>(move);
    }
    GaiaSpawnerMove GetSpawnerMove() const { return m_spawner_move; }
    void PlayPlacerMove(size_t move, Board& board, EventManager& event_manager, size_t tick) {
      Coordinates coords = Coordinates::FromUint64(move);
      Tile& tile = board.GetTile(coords); 
      if (!tile.IsEmptyTile()) {return; }
      if (m_spawner_move == GaiaSpawnerMove::SpawnBomb) {
        tile = Tile::CreateTileWithSpawnedBomb(tick);
	event_manager.AddEventFromTileAtPosition(coords, board);
      } else if (m_spawner_move == GaiaSpawnerMove::SpawnPowerup) {
	tile = Tile::CreateTileWithSpawnedPowerup(tick);
	event_manager.AddEventFromTileAtPosition(coords, board);
      }
    }

  private:
    GaiaSpawnerMove m_spawner_move;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_GAIA_MOVE_PLAYER_HPP_
