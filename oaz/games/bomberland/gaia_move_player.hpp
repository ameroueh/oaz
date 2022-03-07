#ifndef OAZ_GAMES_BOMBERLAND_GAIA_MOVE_PLAYER_HPP_
#define OAZ_GAMES_BOMBERLAND_GAIA_MOVE_PLAYER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"

namespace oaz::games::bomberland {

class GaiaMovePlayer {
  public:
    GaiaMovePlayer(): m_spawner_move(Pass) {}
    void PlaySpawnerMove(size_t move) {
      m_spawner_move = static_cast<GaiaSpawnerMove>(move);
    }
    void PlayPlacerMove(size_t move) {
      
    }

  private:
    GaiaSpawnerMove m_spawner_move;
};
} // namespace oaz::games::bomberland
