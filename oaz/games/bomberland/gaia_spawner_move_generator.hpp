#ifndef OAZ_GAMES_BOMBERLAND_GAIA_SPAWNER_MOVE_GENERATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_GAIA_SPAWNER_MOVE_GENERATOR_HPP_

#include <vector>

namespace oaz::games::bomberland {

enum class GaiaSpawnerMove {
  Pass,
  SpawnBomb,
  SpawnPowerup
};

class GaiaSpawnerMoveGenerator {
  public:
    void operator()(std::vector<size_t>& moves) {
	moves = {
	  static_cast<size_t>(GaiaSpawnerMove::Pass),
	  static_cast<size_t>(GaiaSpawnerMove::SpawnBomb),
	  static_cast<size_t>(GaiaSpawnerMove::SpawnPowerup)
	};
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_GAIA_SPAWNER_MOVE_GENERATOR_HPP_
