#ifndef OAZ_GAMES_BOMBERLAND_PLAYER_STATUS_UPDATER_HPP_
#define OAZ_GAMES_BOMBERLAND_PLAYER_STATUS_UPDATER_HPP_

namespace oaz::games::bomberland {

class PlayerStatusUpdater {
  public:
    PlayerStatusUpdater() {}
    void operator()(boost::multi_array<Agent, 2>& agents, Board& board, size_t tick) {
      for (size_t player=0; player!=1; ++player) {
        for (size_t agent_id=0; agent_id!=2; ++agent_id) {
        Agent& agent = agents[player][agent_id];
	Tile& tile = board.GetTile(agent.GetPosition());
        if (tile.HasFire() || tile.HasBlast()) {
          agent.DealDamage(tick);
        } else if (tile.HasSpawnedBomb()) {
          agent.AddBomb();
	  tile = Tile::CreateEmptyTile();
        } else if (tile.HasSpawnedPowerup()) {
	  agent.IncreaseBlastRadius();
	  tile = Tile::CreateEmptyTile();
        }
        }
      }
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_PLAYER_STATUS_UPDATER_HPP_
