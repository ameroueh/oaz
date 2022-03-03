#ifndef __BOMBERLAND_HPP__
#define __BOMBERLAND_HPP__

#include <memory>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <bitset>
#include <iostream>

#include "oaz/games/game.hpp"

#include "boost/multi_array.hpp"


namespace oaz::games::bomberland {


enum Player {
  Player0Agent0,
  Player0Agent1,
  Player0Agent2,
  Player1Agent0,
  Player1Agent1,
  Player1Agent2,
  GaiaSpawner,
  GaiaSpawnPlacer
};

enum GaiaSpawnerMove {
  Pass,
  SpawnBomb,
  SpawnPowerup
};

class Bomberland : public oaz::games::Game {
  public:

    Bomberland(): 
      m_tick(0),
      m_current_player(0),
      m_gaia_spawner_move(Pass),
      m_pre_move(boost::extents[N_PLAYERS][N_AGENTS_PER_PLAYER]),
      m_player_agents(boost::extents[N_ROWS][N_COLUMNS]),
      m_board(boost::extents[N_ROWS][N_COLUMNS]) {
      std::fill_n(m_board.data(), m_board.num_elements(), Tile::CreateEmptyTile());
      DefaultInitialiseAgents();
    }

    void PlayFromString(std::string) {} // NEEDS IMPLEMENTATION

    void DefaultInitialiseAgents() {
      for(size_t i=0; i!=3; ++i) {
	m_player_agents[0][i] = Agent(TilePosition(0, i));
	m_pre_move[0][i] = TilePosition(0, i);
	m_player_agents[1][i] = Agent(TilePosition(i, 0));
	m_pre_move[1][i] = TilePosition(i, 0);
      }
    }

    float GetScore() const {
      if (m_player0_agents_dead && !m_player1_agents_dead) {
        return -1.0F;
      } else if (!m_player0_agents_dead && m_player1_agents_dead) {
        return 1.0F;
      } else {
        return 0.0F;
      }
    }

    size_t GetCurrentPlayer() const {
      return m_current_player;
    }

    bool IsFinished() const {
      return m_tick == 300 || m_player0_agents_dead || m_player1_agents_dead; 
    }

    void WriteStateToTensorMemory(float* pointer) const {} // NEEDS IMPLEMENTATIION
    void WriteCanonicalStateToTensorMemory(float* pointer) const {} // NEEDS IMPLEMENTATIION
    void InitialiseFromState(float* pointer) {} // NEEDS IMPLEMENTATION
    void InitialiseFromCanonicalState(float* pointer) {} // NEEDS IMPLEMENTATION

    std::unique_ptr<Game> Clone() const {
      return std::make_unique<Bomberland>(*this);
    }

    void GetAvailableMoves(std::vector<size_t>* moves) const {
      moves->clear();
      switch (GetCurrentPlayer()) {
	      case Player0Agent0:
		GetAgentMoves(0, moves);
		break;
	      case Player0Agent1:
		GetAgentMoves(0, moves);
		break;
	      case Player0Agent2:
		GetAgentMoves(0, moves);
		break;
	      case Player1Agent0:
		GetAgentMoves(1, moves);
		break;
	      case Player1Agent1:
		GetAgentMoves(1, moves);
		break;
	      case Player1Agent2:
		GetAgentMoves(1, moves);
		break;
	      case GaiaSpawner:
		GetGaiaSpawnerMoves(moves);
		break;
	      case GaiaSpawnPlacer:
		GetGaiaSpawnPlacerMoves(moves);
		break;
      }
    }

    void PlayMove(size_t move) {
      AddFire();
      switch (GetCurrentPlayer()) {
	      case Player0Agent0:
		PlayAgentMove(0, 0, move);
		break;
	      case Player0Agent1:
		PlayAgentMove(0, 1, move);
		break;
	      case Player0Agent2:
		PlayAgentMove(0, 2, move);
		break;
	      case Player1Agent0:
		PlayAgentMove(1, 0, move);
		break;
	      case Player1Agent1:
		PlayAgentMove(1, 1, move);
		break;
	      case Player1Agent2:
		PlayAgentMove(1, 2, move);
		break;
	      case GaiaSpawner:
		PlayGaiaSpawnerMove(move);
		break;
	      case GaiaSpawnPlacer:
		PlayGaiaSpawnPlacerMove(move);
		break;
      }
      ResolvePlayerMoves();
      UpdateCurrentPlayer();
      if (m_current_player == 8) {
        AdjudicateTurn();
	UpdateCurrentPlayer();
      }
    }

    struct Class : public Game::Class {
      size_t GetMaxNumberOfMoves() const override { return N_ROWS*N_COLUMNS; }
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
        const std::vector<int> m_board_shape{150, 150};
    };

    const Class& ClassMethods() const override { return Class::Methods(); }

  private:
    size_t constexpr GetNColumns() const {
      return N_COLUMNS;
    }
    size_t constexpr GetNRows() const {
      return N_ROWS;
    }
    bool IsWithinBounds(TilePosition pos) const {
      return pos.first()< GetNColumns() && pos.second() < GetNRows();
    }
    void AddFire() {}

    void MoveAgent(size_t player, size_t agent_id, TilePosition position) {
        Agent& agent = m_player_agents[player][agent_id];
	agent.SetPosition(position);
	Tile& tile = m_board[position.first()][position.second()];
        if (tile.HasSpawnedBomb()) {
          agent.AddBomb();
	  tile = Tile::CreateEmptyTile();
	} else if ( tile.HasSpawnedPowerup() ) {
	  agent.IncreaseBlastRadius();
	  tile = Tile::CreateEmptyTile();
	}
    }

    void ResolvePlayerMoves() {
    }

    Agent& GetAgentFromCurrentPlayer(size_t current_player) {
      return m_player_agents[current_player / 3][current_player % 3];
    }

    void AdjudicateTurn() {
      for(size_t player=0; player!=2; ++player) {
        for(size_t agent_id=0; agent_id!=3; ++agent_id) {
	  Agent& agent = GetAgent(player, agent_id);
	  if (!agent.IsDead()) {
	    TilePosition position = agent.GetPosition();
	    Tile& tile = m_board[position.first()][position.second()];
	    if (tile.HasBlast() || tile.HasFire()) {
	      agent.DecreaseHealth();
	      agent.SetInvulnerable(m_tick);
	    }
	  }
        }
      }
    }

    void UpdateCurrentPlayer() {
      if (m_current_player == 6 && m_gaia_spawner_move != Pass) {
        m_current_player = 7;
      } else {
        if (m_current_player == 6) {
          m_current_player = 0;
        } else {
          ++m_current_player;
        }
        while (m_current_player < 6 && GetAgentFromCurrentPlayer(m_current_player).IsDead()) {
	  ++m_current_player;
        }
      }
    }

    void PlayGaiaSpawnPlacerMove(size_t move) {
      TilePosition position = DecodePosition(move);
      Tile& tile = m_board[position.first()][position.second()];
      switch (m_gaia_spawner_move) {
        case SpawnBomb:
	  tile = Tile::CreateTileWithSpawnedBomb(m_tick);
	  break;
	case SpawnPowerup:
	  tile = Tile::CreateTileWithSpawnedPowerup(m_tick);
	  break;
      }
    }

    void PlayGaiaSpawnerMove(size_t move) {
      switch (move) {
        case 0:
	  m_gaia_spawner_move = Pass;
	  break;
	case 1:
	  m_gaia_spawner_move = SpawnBomb;
	  break;
	case 2:
	  m_gaia_spawner_move = SpawnPowerup;
	  break;
      }
    }

    size_t GetAgentMoves(size_t player, std::vector<size_t>* moves) const {
      moves->push_back(0); // PASS
      moves->push_back(1); // UP
      moves->push_back(2); // DOWN
      moves->push_back(3); // LEFT
      moves->push_back(4); // RIGHT
      moves->push_back(5); // PLACE_BOMB
      auto bomb_positions = m_player_bomb_positions[player];
      for (auto it = bomb_positions.cbegin(); it != bomb_positions.cend(); ++it) {
        moves->push_back((EncodePosition(*it) << 4) | 0b1111); // DETONATE_BOMB_AT_POSITION 
      }
    }

    void PlayAgentMove(size_t player, size_t agent, size_t move) {
      switch (move) {
        case 0:
	  break;
        case 1:
	  PreMoveAgent(player, agent, TilePosition(1, 0));
	  break;
        case 2:
	  PreMoveAgent(player, agent, TilePosition(-1, 0));
	  break;
        case 3:
	  PreMoveAgent(player, agent, TilePosition(0, 1));
	  break;
        case 4:
	  PreMoveAgent(player, agent, TilePosition(0, -1));
	  break;
	case 5:
	  PlaceBomb(player, agent);
	  break;
	default:
	  DetonatePlayerBomb(player, DecodePosition(move >> 4));
      }
    }

    void PlaceBomb(size_t player, size_t agent_id) {
      Agent& agent = m_player_agents[player][agent_id];
      if (agent.GetNBombs() == 0) {
        return;
      }
      size_t blast_radius = agent.GetBlastRadius();
      TilePosition position = agent.GetPosition();
      m_board[position.first()][position.second()] = Tile::CreateTileWithPlacedBomb(player, m_tick + 40, blast_radius);
      m_player_bomb_positions[player].insert(position);
    }

    void DetonatePlayerBomb(size_t player, TilePosition position) {
      Tile& tile = m_board[position.first()][position.second()];
      if (!tile.HasPlacedBomb()) {
        return;
      }
      Bomb bomb = tile.GetBomb();
      if (bomb.GetOwner() != player) {
	return;
      }
      tile = Tile::CreateEmptyTile();
      DetonateBomb(position);
    }


    Agent& GetAgent(size_t player, size_t agent_id) {
      return m_player_agents[player][agent_id];
    }

    void GetGaiaSpawnerMoves(std::vector<size_t>* moves) const {
      moves->push_back(0); // PASS
      moves->push_back(1); // SPAWN BOMB
      moves->push_back(2); // SPAWN POWERUP
    }

    void GetGaiaSpawnPlacerMoves(std::vector<size_t>* moves) const {
      for (auto it = m_free_tiles.cbegin(); it != m_free_tiles.cend(); ++it) {
        moves->push_back(EncodePosition(*it));
      }
    }

    size_t EncodePosition(TilePosition position) const {
      return (position.first() << 4) | position.second();
    }

    TilePosition DecodePosition(size_t encoded_position) const {
      return TilePosition(encoded_position >> 4, encoded_position & 0b1111);
    }
    
    void RemoveSpawnedItems(size_t tick) {
      EmptyExpiredTiles(tick, m_spawn_positions);
    }

    void RemoveBlast(size_t tick) {
      EmptyExpiredTiles(tick, m_blast_positions);
    }
    
    bool EmptyExpiredTiles(size_t tick, std::queue<TilePosition>& queue) {
      while (!queue.empty()) {
	TilePosition position = queue.front();
        Tile& tile = m_board[position.first()][position.second()];
	if (tile.GetExpiryTime() <= tick) {
	  tile = Tile::CreateEmptyTile();
	  queue.pop();
	} else {
	  break;
	}
      }
    }

    static constexpr size_t N_AGENTS_PER_PLAYER = 3;
    static constexpr size_t N_PLAYERS = 2;

    boost::multi_array<Tile, 2> m_board;

    using TilePositionSet = std::unordered_set<TilePosition, TilePositionHasher, TilePositionComparator>;
    TilePositionSet m_free_tiles;
    boost::multi_array<TilePositionSet, 1> m_player_bomb_positions;

    size_t m_current_player;

    GaiaSpawnerMove m_gaia_spawner_move;
    
    std::queue<TilePosition> m_blast_positions;
    std::queue<TilePosition> m_spawn_positions;

    boost::multi_array<Agent, 2> m_player_agents;
    boost::multi_array<TilePosition, 2> m_pre_move;

    size_t m_tick;

    boost::multi_array<size_t, 1> m_n_alive_agents;
    
    static constexpr size_t N_COLUMNS = 15;
    static constexpr size_t N_ROWS = 15;
    
};
}  // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_HPP_
