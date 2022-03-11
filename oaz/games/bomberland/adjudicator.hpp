#ifndef OAZ_GAMES_BOMBERLAND_ADJUDICATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_ADJUDICATOR_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/agent.hpp"
#include "oaz/games/bomberland/gaia_move_player.hpp"
#include "oaz/games/bomberland/player_status_updater.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/fire_adder.hpp"

namespace oaz::games::bomberland {

enum class Player {
  Player0Agent0,
  Player0Agent1,
  Player0Agent2,
  Player1Agent0,
  Player1Agent1,
  Player1Agent2,
  GaiaSpawner,
  GaiaPlacer
};

class Adjudicator {
  public:
    Adjudicator():
      m_current_player(Player::Player0Agent0),
      m_player0_is_dead(false),
      m_player1_is_dead(false) {}

    void Update(
      boost::multi_array<Agent, 2>& agents,
      Board& board,
      size_t& tick,
      GaiaMovePlayer& gaia_move_player,
      EventManager& event_manager,
      FireAdder& fire_adder
    ) {
      if (GameIsFinished()) { return; }
      m_player0_is_dead = agents[0][0].IsDead() && agents[0][1].IsDead() && agents[0][2].IsDead();
      m_player1_is_dead = agents[1][0].IsDead() && agents[1][1].IsDead() && agents[1][2].IsDead();

      if (GameIsFinished()) { return; }

      size_t previous_player = static_cast<size_t>(m_current_player);
      m_current_player = UpdatePlayer(m_current_player, agents, gaia_move_player);
      if (previous_player >= static_cast<size_t>(m_current_player)) {
	EndTurn(agents, board, event_manager, fire_adder, tick);
      }
    }
    bool GameIsFinished() const {
      return m_player0_is_dead || m_player1_is_dead; 
    }

    float GetScore() const {
      if (!GameIsFinished()) { return 0.0F; }
      else if (m_player1_is_dead) { return 1.0F; }
      else { return -1.0F; }
    }

    Player GetCurrentPlayer() const { return m_current_player; }

  private:
    Player UpdatePlayer(Player player, boost::multi_array<Agent, 2>& agents, GaiaMovePlayer& gaia_move_player) const {
      player = IncrementPlayer(player, gaia_move_player);
      while (static_cast<size_t>(player) < 6 && IsDead(player, agents)) {
        player = IncrementPlayer(player, gaia_move_player);
      }
      return player;
    }
    Player IncrementPlayer(Player player, GaiaMovePlayer& gaia_move_player) const {
      if (player == Player::GaiaSpawner
	&& gaia_move_player.GetSpawnerMove() == GaiaSpawnerMove::Pass
	|| player == Player::GaiaPlacer) { return Player::Player0Agent0; }
      return static_cast<Player>(static_cast<size_t>(player) + 1);
    }
    bool IsDead(Player player, boost::multi_array<Agent, 2>& agents) const {
      size_t cplayer = static_cast<size_t>(player) / 3;
      size_t agent_id = static_cast<size_t>(player) % 3;
      return agents[cplayer][agent_id].IsDead();
    }
    void EndTurn(
	boost::multi_array<Agent, 2>& agents,
	Board& board,
	EventManager& event_manager,
	FireAdder& fire_adder,
	size_t& tick) {
      if (tick >= 300 && tick % 2 == 0) { fire_adder(board, event_manager, tick); }
      PlayerStatusUpdater()(agents, board, tick);
      event_manager.ClearEvents(board, tick);
      ++tick;
    }

    Player m_current_player;
    bool m_player0_is_dead;
    bool m_player1_is_dead;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_ADJUDICATOR_HPP_
