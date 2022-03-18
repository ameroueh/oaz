#ifndef __BOMBERLAND_HPP__
#define __BOMBERLAND_HPP__

#include "boost/multi_array.hpp"

#include "oaz/games/game.hpp"

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/adjudicator.hpp"
#include "oaz/games/bomberland/agent_move_generator.hpp"
#include "oaz/games/bomberland/agent_move_player.hpp"
#include "oaz/games/bomberland/gaia_spawner_move_generator.hpp"
#include "oaz/games/bomberland/gaia_placer_move_generator.hpp"
#include "oaz/games/bomberland/fire_adder.hpp"
#include "oaz/games/bomberland/default_agent_initializer.hpp"
#include "oaz/games/bomberland/position_resolver.hpp"
#include "oaz/games/bomberland/bomb_list_cleaner.hpp"

namespace oaz::games::bomberland {

struct GameConfig {
  GameConfig():
	ammo_duration_ticks(40),
	bomb_duration_ticks(40),
	blast_duration_ticks(10),
	bomb_armed_ticks(5),
	fire_spawn_interval_ticks(2),
	game_duration_ticks(300),
	invulnerability_ticks(5)
  {} 

  GameConfig(
    size_t ammo_duration_ticks,
    size_t bomb_duration_ticks,
    size_t blast_duration_ticks,
    size_t bomb_armed_ticks,
    size_t fire_spawn_interval_ticks,
    size_t game_duration_ticks,
    size_t invulnerability_ticks
  ):
    ammo_duration_ticks(ammo_duration_ticks),
    bomb_duration_ticks(bomb_duration_ticks),
    blast_duration_ticks(blast_duration_ticks),
    bomb_armed_ticks(bomb_armed_ticks),
    fire_spawn_interval_ticks(fire_spawn_interval_ticks),
    game_duration_ticks(game_duration_ticks),
    invulnerability_ticks(invulnerability_ticks)
  {}
    
  size_t ammo_duration_ticks;
  size_t bomb_duration_ticks;
  size_t blast_duration_ticks;
  size_t bomb_armed_ticks;
  size_t fire_spawn_interval_ticks;
  size_t game_duration_ticks;
  size_t invulnerability_ticks;
};

class Bomberland : public oaz::games::Game {
  public:
    
    Bomberland():
      m_tick(0),
      m_agents(boost::extents[2][3]),
      m_player_bombs(boost::extents[2])
      {
         DefaultAgentInitializer()(m_agents);	
      }
    
    Bomberland(const GameConfig& game_config):
      m_tick(0),
      m_agents(boost::extents[2][3]),
      m_player_bombs(boost::extents[2]),
      m_adjudicator(
	game_config.game_duration_ticks,
	game_config.fire_spawn_interval_ticks,
	game_config.invulnerability_ticks),
      m_gaia_move_player(game_config.ammo_duration_ticks),
      m_game_config(game_config)
      {
         DefaultAgentInitializer()(m_agents);	
      }
    
    float GetScore() const {
      return m_adjudicator.GetScore();
    }

    size_t GetCurrentPlayer() const {
      return static_cast<size_t>(m_adjudicator.GetCurrentPlayer()); }

    bool IsFinished() const {
      return m_adjudicator.GameIsFinished();
    }
    
    std::unique_ptr<Game> Clone() const {
      return std::make_unique<Bomberland>(*this);
    }
    
    void GetAvailableMoves(std::vector<size_t>* moves) const {
      using AgentMoveGeneratorVecBombIterator = AgentMoveGenerator<std::vector<Coordinates>::const_iterator>;
      switch (m_adjudicator.GetCurrentPlayer()) {
	      case Player::Player0Agent0:
		BombListCleaner()(0, m_player_bombs[0], m_board);
		AgentMoveGeneratorVecBombIterator()(0, *moves, m_player_bombs[0].cbegin(), m_player_bombs[0].cend());
		break;
	      case Player::Player0Agent1:
		BombListCleaner()(0, m_player_bombs[0], m_board);
		AgentMoveGeneratorVecBombIterator()(0, *moves, m_player_bombs[0].cbegin(), m_player_bombs[0].cend());
		break;
	      case Player::Player0Agent2:
		BombListCleaner()(0, m_player_bombs[0], m_board); AgentMoveGeneratorVecBombIterator()(0, *moves, m_player_bombs[0].cbegin(), m_player_bombs[0].cend()); break; case Player::Player1Agent0: BombListCleaner()(1, m_player_bombs[1], m_board);
		AgentMoveGeneratorVecBombIterator()(1, *moves, m_player_bombs[1].cbegin(), m_player_bombs[1].cend());
		break;
	      case Player::Player1Agent1:
		BombListCleaner()(1, m_player_bombs[1], m_board);
		AgentMoveGeneratorVecBombIterator()(1, *moves, m_player_bombs[1].cbegin(), m_player_bombs[1].cend());
		break;
	      case Player::Player1Agent2:
		BombListCleaner()(1, m_player_bombs[1], m_board);
		AgentMoveGeneratorVecBombIterator()(1, *moves, m_player_bombs[1].cbegin(), m_player_bombs[1].cend());
		break;
	      case Player::GaiaSpawner:
		GaiaSpawnerMoveGenerator()(*moves);
		break;
	      case Player::GaiaPlacer:
		GaiaPlacerMoveGenerator()(*moves, m_board);
		break;
      }
    }
    
    void PlayMove(size_t move) {
      switch (m_adjudicator.GetCurrentPlayer()) {
	      case Player::Player0Agent0:
		AgentMovePlayer()(0, 0, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::Player0Agent1:
		AgentMovePlayer()(0, 1, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::Player0Agent2:
		AgentMovePlayer()(0, 2, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::Player1Agent0:
		AgentMovePlayer()(1, 0, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::Player1Agent1:
		AgentMovePlayer()(1, 1, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::Player1Agent2:
		AgentMovePlayer()(1, 2, move, m_tick, m_board, m_agents, m_position_resolver, m_event_manager, m_game_config.bomb_duration_ticks, m_game_config.bomb_armed_ticks, m_game_config.blast_duration_ticks);
		break;
	      case Player::GaiaSpawner:
		m_gaia_move_player.PlaySpawnerMove(move);
		break;
	      case Player::GaiaPlacer:
		m_gaia_move_player.PlayPlacerMove(move, m_board, m_event_manager, m_tick);
		break;
      }
      m_adjudicator.Update(m_agents, m_board, m_tick, m_game_config.blast_duration_ticks, m_gaia_move_player, m_event_manager, m_fire_adder);
    }

    /* START DUMMY IMPLEMENTATIONS */
    void PlayFromString(std::string) {}
    void WriteStateToTensorMemory(float* pointer) const {}
    void WriteCanonicalStateToTensorMemory(float* pointer) const {}
    void InitialiseFromState(float* pointer) {}
    void InitialiseFromCanonicalState(float* pointer) {}
    
    struct Class : public Game::Class {
      size_t GetMaxNumberOfMoves() const override { return 225; }
      const std::vector<int>& GetBoardShape() const override {
	return m_board_shape;
      }
      GameMap* CreateGameMap() const override {
        return nullptr;
      }
      static const Class& Methods() {
        static const Class meta;
	return meta;
      }
      private:
        const std::vector<int> m_board_shape{15, 15};
    };

    const Class& ClassMethods() const override { return Class::Methods(); }

    /* END DUMMY IMPLEMENTATIONS */

  private:
    friend class JsonStateBuilder;

    size_t m_tick;
    boost::multi_array<Agent, 2> m_agents;
    PositionResolver m_position_resolver;
    Board m_board;
    Adjudicator m_adjudicator;
    GaiaMovePlayer m_gaia_move_player;
    EventManager m_event_manager;
    FireAdder m_fire_adder;
    mutable boost::multi_array<std::vector<Coordinates>, 1> m_player_bombs;

    // Configs
    GameConfig m_game_config;
};
}  // namespace oaz::games::bomberland

#include "oaz/games/bomberland/event_manager_impl.hpp"
#endif // OAZ_GAMES_BOMBERLAND_HPP_
