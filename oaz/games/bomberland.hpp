#ifndef __BOMBERLAND_HPP__
#define __BOMBERLAND_HPP__

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace oaz::games::bomberland {

using Tile = size_t; 
using TilePosition = std::pair<size_t, size_t>;

enum Player {
  Player0Agent0,
  Player0Agent1,
  Player0Agent2,
  Player1Agent0,
  Player1Agent1,
  Player1Agent2,
  GaiaSpawner,
  GaiaSpawnPlacer
}

enum GaiaSpawnerMove {
  Pass,
  SpawnBomb,
  SpawnPowerup;
};

class Bomb {
  public:
    Bomb(size_t owner, size_t blast_radius): m_owner(owner), m_blast_radius(blast_radius) {}
    size_t GetOwner() const { return m_owner; }
    size_t GetBlastRadius() const { return m_blast_radius; }
  private:
    size_t m_owner;
    size_t m_blast_radius;
};

class Tile {
  public:
    bool IsWalkable() const {
      return GetValue(WALKABLE_OFFSET, WALKABLE_SIZE) == 0;
    }

    bool IsInvulnerable() const {
      return GetValue(INVULNERABLE_OFFSET, INVULNERABLE_SIZE) == 1;
    }

    bool GetHP() const {
      return GetValue(HP_OFFSET, HP_SIZE);
    }

    size_t GetExpiryTime() const {
      return GetValue(EXPIRY_OFFSET, EXPIRY_SIZE);
    }

    size_t GetUnitId() const {
      return GetValue(UNIT_ID_OFFSET, UNIT_ID_SIZE);
    }

    size_t GetTileType() const {
      return GetValue(TILE_TYPE_OFFSET, TILE_TYPE_SIZE);
    }

    Bomb GetBomb() const {
      return Bomb(GetValue(OWNER_OFFSET, OWNER_SIZE), GetValue(BLAST_RADIUS_OFFSET, BLAST_RADIUS_SIZE));
    }

    bool HasBomb() const {
      return GetTileType() == 1;
    }

    bool HasBlast() const {
      return GetTileType() == 4;
    }

    bool HasFire() const {
      return GetTileType() == 5;
    }
      
  private:
    
    size_t GetValue(size_t offset, size_t bit_size) const {
      return (m_tile >> offset) & ((1 << bit_size) - 1);
    }

    void SetValue(size_t value, size_t offset, size_t bit_size) const {
      uint32_t mask = ~(((1 << bit_size) - 1) << offset);
      m_tile &= mask;
      m_tile |= (value << offset);
    }

    static constexpr WALKABLE_OFFSET = 0;
    static constexpr WALKABLE_SIZE = 1;

    # Walkable tile attributes
    static constexpr TILE_TYPE_OFFSET = 1;
    static constexpr TILE_TYPE_SIZE = 3; // 0 = EMPTY, 1 = PLACED_BOMB, 2 = SPAWNED_BOMB, 3 = POWERUP, 4 = BLAST, 5 = FIRE 
    static constexpr EXPIRY_OFFSET = 4;
    static constexpr EXPIRY_SIZE = 9;
    static constexpr UNIT_ID_OFFSET = 13;
    static constexpr UNIT_ID_SIZE = 9;
    static constexpr OWNER_OFFSET = 22;
    static constexpr OWNER_SIZE = 1;
    static constexpr BLAST_RADIUS_OFFSET = 23;
    static constexpr BLAST_RADIUS_SIZE = 4;


    # Non-walkable tile attributes
    static constexpr INVULNERABLE_OFFSET = 1;
    static constexpr INVULNERABLE_SIZE = 1;
    static constexpr HP_OFFSET = 2;
    static_constexpr HP_SIZE = 2;

    uint32_t m_tile;
};


class Board {
  public:
    Board(): m_board(boost::extents[N_COLUMNS][N_ROWS]) {}
    size_t constexpr GetNColumns() const {
      return N_COLUMNS;
    }
    size_t constexpr GetNRows() const {
      return N_ROWS;
    }
    bool IsWithinBounds(TilePosition pos) const {
      return pos.first< GetNColumns() && pos.second < GetNRows();
    }
    Tile& GetTile(TilePosition pos) {
      return m_board[pos.first][pos.second];
    }

  private:
    static constexpr size_t N_COLUMNS = 15;
    static constexpr size_t N_ROWS = 15;
    
    boost::multi_array<Tile, 2> m_board;
};

class BomberlandAgent {
  public:
    BomberlandAgent(TilePosition);

    size_t GetHealth() const { return m_health; }
    size_t GetBlastRadius() const { return m_blast_radius; }
    size_t GetNBombs() const { return m_n_bombs; }
    bool IsDead() const { return m_health == 0; }
    bool IsInvulnerable(size_t tick) const {
      return m_invulnerability_time < INVULNERABILITY_TIME;
    }

    void SetInvulnerabilityTime(size_t invulnerability_time) {
      m_invulnerability_time = invulnerability_time;
    }

    void DecreaseHealth() {
      m_health = m_health > 0 ? m_health - 1 : 0;
    } 
    void UseBomb() {
      m_n_bombs = m_n_bombs > 0 ? m_n_bombs - 1 : 0;
    }
    void IncreaseBlastRadius() {
      ++m_blast_radius;
    }

  private:
    static constexpr INVULNERABILITY_TIME = 5;
    size_t m_blast_radius;
    size_t m_health;
    size_t m_n_bombs;
    size_t m_invulnerability_time;
    TilePosition m_pos;
}; 

class Bomberland : public Game {
  public:
    Bomberland(); 

    void GetAvailableMoves(std::vector<size_t>* moves) const {
      switch(GetCurrentPlayer()) {
	      case Player.Player0Agent0:
		GetEncodedAgentMoves(0, 0, moves);
		break;
	      case Player.Player0Agent1:
		GetEncodedAgentMoves(0, 1, moves);
		break;
	      case Player.Player0Agent2:
		GetEncodedAgentMoves(0, 2, moves);
		break;
	      case Player.Player1Agent0:
		GetEncodedAgentMoves(1, 0, moves);
		break;
	      case Player.Player1Agent1:
		GetEncodedAgentMoves(1, 1, moves);
		break;
	      case Player.Player1Agent2:
		GetEncodedAgentMoves(1, 2, moves);
		break;
	      case Player.GaiaSpawner:
		GetGaiaSpawnerMoves(moves);
		break;
	      case Player.GaiaSpawnPlacer:
		GetGaiaSpawnPlacerMoves(moves);
		break;
      }
    }

    void PlayMove(size_t move) {
      AddFire();
      switch(GetCurrentPlayer()) {
	      case Player.Player0Agent0:
		PlayEncodedAgentMove(0, 0, move);
		break;
	      case Player.Player0Agent1:
		PlayEncodedAgentMove(0, 1, move);
		break;
	      case Player.Player0Agent2:
		PlayEncodedAgentMove(0, 2, move);
		break;
	      case Player.Player1Agent0:
		PlayEncodedAgentMove(1, 0, move);
		break;
	      case Player.Player1Agent1:
		PlayEncodedAgentMove(1, 1, move);
		break;
	      case Player.Player1Agent2:
		PlayEncodedAgentMove(1, 2, move);
		break;
	      case Player.GaiaSpawner:
		PlayGaiaSpawnerMove(move);
		break;
	      case Player.GaiaSpawnPlacer:
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

  private:
    void AddFire() {}

    void ResolvePlayerMoves() {} // If  

    void AdjudicateTurn() {
      for(size_t player=0; player!=2; ++player) {
        for(size_t agent=0; agent!=3; ++agent) {
	  PlayerAgent& agent = GetAgent(player, agent);
	  if (!agent.IsDead()) {
	    Tile& tile = m_board.GetTile(agent.GetPosition());
	    if (tile.HasBlast() || tile.HasFire()) {
	      agent.DecreaseHealth();
	    }
	  }
        }
      }
    }

    void UpdateCurrentPlayer() {
      if (m_current_player == 6 && m_gaia_spawner_move != GaiaSpawnerMove.Pass) {
        m_current_player = 7;
      } else {
        if (m_current_player == 6) {
          m_current_player = 0;
        } else {
          ++m_current_player;
        }
        while(m_current_player < 6 && GetAgent(m_current_player).IsDead()) {
	  ++m_current_player;
        }
      }
    }

    void PlayGaiaSpawnPlacerMove(size_t move) {
      TilePosition position = DecodePosition(move);
      Tile& tile = m_board.GetTile(position);
      switch(m_gaia_spawner_move) {
        case GaiaSpawnerMove.SpawnBomb:
	  tile = NewTileWithSpawnedBomb();
	  break;
	case GaiaSpawnPlacerMove.SpawnPowerup:
	  tile = NewTileWithSpawnedPowerup();
	  break;
      }
    }

    void PlayGaiaSpawnerMove(size_t move) {
      switch (move) {
        case 0:
	  m_gaia_spawner_move = GaiaSpawnerMove.Pass;
	  break;
	case 1:
	  m_gaia_spawner_move = GaiaSpawnerMove.SpawnBomb;
	  break;
	case 2:
	  m_gaia_spawner_move = GaiaSpawnerMove.SpawnPowerup;
	  break;
      }
    }

    size_t GetEncodedAgentMoves(size_t player, size_t agent, std::vector<size_t>* moves) const {
      moves->clear();
      moves->push_back(0); // PASS
      moves->push_back(1); // UP
      moves->push_back(2); // DOWN
      moves->push_back(3); // LEFT
      moves->push_back(4); // RIGHT
      moves->push_back(5); // PLACE_BOMB
      auto* bomb_positions = m_board.GetAgentBombPositions(player);
      for(auto it = bomb_positions->cbegin(); it != bomb_positions->cend(); ++it) {
        moves->push_back((EncodePosition(*it) << 4) | 0b1111); // DETONATE_BOMB_AT_POSITION 
      }
    }

    void PlayEncodedAgentMove(size_t player, size_t agent, size_t move) {
      switch(move) {
        case 0:
	  break;
        case 1:
	  MoveAgent(player, agent, TilePosition(1, 0));
	  break;
        case 2:
	  MoveAgent(player, agent, TilePosition(-1, 0));
	  break;
        case 3:
	  MoveAgent(player, agent, TilePosition(0, 1));
	  break;
        case 4:
	  MoveAgent(player, agent, TilePosition(0, -1));
	  break;
	case 5:
	  PlaceBomb(player, agent);
	  break;
	default:
	  DetonatePlayerBomb(player, DecodePosition(move >> 4));
      }
    }

    void PlaceBomb(size_t player, size_t agent) {
      PlayerAgent& agent = GetAgent(player, agent);
      if (agent.GetNBombs() == 0) {
        break;
      }
      size_t blast_radius = agent.GetBlastRadius();
      TilePosition position = agent.GetPosition();
      m_board.GetTile(position) = NewTileWithBomb(player, m_tick + 40, blast_radius);
      m_player_bombs[player].push_back(position);
    }

    void DetonatePlayerBomb(size_t player, size_t position) {
      Tile& tile = m_board.GetTile(position);
      if (!tile.HasBomb()) {
        break;
      }
      Bomb bomb = tile.GetBomb();
      if (bomb.GetOwner() != player) {
	break;
      }
      tile = NewEmptyTile();
      AddBlast(position, blast_radius, m_tick + 5);
    }
 
    void DetonateBomb(TilePosition position) {
      size_t blast_radius = bomb.GetBlastRadius();
      Tile& = m_board.GetTile(position);   
      tile.AddBlast();
      AddBlastAlongLine(position, blast_radius, TilePosition(1, 0));
      AddBlastAlongLine(position, blast_radius, TilePosition(-1, 0));
      AddBlastAlongLine(position, blast_radius, TilePosition(0, 1));
      AddBlastAlongLine(position, blast_radius, TilePosition(0, -1));
    }

    void AddBlastAlongLine(TilePosition position, size_t blast_radius, TilePosition vector) {
      TilePosition cposition = position + vector;
      for (size_t i=0; i!=blast_radius-1; ++i) {
	if(!m_board.IsWithinBounds(cposition))
	  break;
        Tile& tile = m_board.GetTile(cposition);
        if(!tile.IsWalkable()) {
	  tile.DecreaseHealth();
	  break;
	}
	tile.AddBlast();
	cposition += vector;
      }
    }

    void MoveAgent(size_t player, size_t agent, TilePosition vector) {
      PlayerAgent& agent = GetAgent(player, agent);
      TilePosition position = agent.GetPosition();
      TilePosition new_position = position + vector;
      if(m_board.IsWithinBounds(new_position) && m_board.GetTile(new_position).IsWalkable()) {
	Tile& tile = m_board.GetTile(new_position);
        if(tile.HasSpawnedBomb()) {
          agent.AddBomb();
	  tile = NewEmptyTile();
	} else if ( tile.HasPowerUp() ) {
	  agent.IncreaseBlastRadius();
	  tile = NewEmptyTile();
	}
        agent.SetPosition(new_position);
       }
    }

    PlayAgent& GetAgent(size_t player, size_t agent) {
      if(player == 0) {
        return m_player0_agents[agent];
      else {
        return m_player1_agents[agent];
      }
    }

    void GetGaiaSpawnerMoves(vector<size_t>* moves) const {
      moves->clear();
      moves->push_back(0); // PASS
      moves->push_back(1); // SPAWN BOMB
      moves-> push_back(2); // SPAWN POWERUP
    }

    void GetGaiaSpawnPlacerMoves(vector<size_t>* moves) const {
      moves->clear();
      for(auto it = m_free_tiles->cbegin(); it != m_free_tiles->cend(); ++it) {
        moves->push_back(EncodePosition(*it));
      }
    }

    size_t EncodePosition(TilePosition position) const {
      return (position.first << 4) | position.second;
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
      while(!queue.empty() && GetTile(queue.top()).GetExpiry() <= tick) {
        GetTile(queue.top()) = NewEmptyTile();
	queue.pop();
      }
    }

    bool TileIsFree(TilePosition) const;
    void PlayAgentMove(size_t, size_t, size_t); 

    static constexpr size_t N_AGENTS_PER_PLAYER = 3;

    boost::multi_array<Tile, 2> m_board;
    std::unordered_map<TilePosition> m_empty_tiles;

    std::vector<BomberlandAgent> m_agents;
    std::vector<TilePosition> m_pre_positions;
    std::unordered_set m_free_tiles;

    size_t m_current_player;
    Board m_board;

    GaiaSpawnerMove m_gaia_spawner_move;
    
    std::queue<TilePosition> m_blast_positions;
    std::queue<TilePosition> m_spawn_positions;
    std::queue<TilePosition> m_agent0_bomb_positions;
    std::queue<TilePosition> m_agent1_bomb_positions;
};
}  // namespace oaz::games::bomberland
#endif
