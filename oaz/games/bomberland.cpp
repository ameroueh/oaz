#include "oaz/games/bomberland.hpp"

using namespace oaz::games::bomberland;

BomberlandAgent::BomberlandAgent(TilePosition pos):
  m_blast_radius(3),
  m_n_bombs(3),
  m_health(3),
  m_pos(pos) {}

void BomberlandAgent::DecreaseHealth() {
  m_health = m_health > 0 ? m_health - 1 : 0;
}

void BomberlandAgent::UseBomb() {
  m_n_bombs = m_n_bombs > 0 ? m_n_bombs - 1 : 0;
}

void BomberlandAgent::IncreaseBlastRadius() {
  ++m_blast_radius;
}

bool BomberlandAgent::IsDead() const {
  return m_health == 0;
}


Bomberland::Bomberland():
  m_board(boost::extents[N_ROWS][N_COLUMNS]),
  m_player0_agents(3, BomberlandAgent()),
  m_current_player(0) {}

void Bomberland::PlayAgentMove(size_t move, BomberlandAgent& agent) {
  
  switch(DecodeMove(move)) {
    case Move.UP:
      PreMove(agent, ComputePosition(agent.GetPosition(), TilePosition(1, 0));
      break;
    case Move.DOWN:
      PreMove(agent, ComputePosition(agent.GetPosition(), TilePosition(-1, 0));
      break;
    case Move.LEFT:
      PreMove(agent, ComputePosition(agent.GetPosition(), TilePosition(0, -1));
      break;
    case Move.RIGHT:
      PreMove(agent, ComputePosition(agent.GetPosition(), TilePosition(0, 1));
      break;
    case Move.PLACE_BOMB:
      PlaceBomb(agent);
      break;
    case Move.DETONATE_BOMB:
      size_t bomb_id = DecodeBombId(move);
      DetonateBomb(agent, DecodeBombId(move));
  }
}

void Bomberland::DetonateBomb(agent, bomb_id) {
  Bomb& bomb = m_bombs[bomb_id];
  TilePosition pos = bomb.GetPosition();
  size_t blast_radius = bomb.GetBlastRadius();

  TilePosition current_position(pos);
  for(size_t i=0; i!=blast_radius; ++i) {
    TilePosition current_position = ComputePosition(current_position, TilePosition(1, 0));
    if(TileHasObstacle(current_position)) {
      DecreaseObstacleHealth(current_position);
      MaybeDeleteObstacle(current_position);
      break;
    } else {
      AddBlast(current_position);
    }
  }
}

void Bomberland::PreMove(BomberlandAgent& agent, TilePosition& position) {
  m_pre_positions[agent.GetId()] = position;
}

void Bomberland::PlayMove(size_t move) {
  if(GetCurrentPlayer() < 2 * N_AGENTS_PER_PLAYER) {
    PlayAgentMove(move, m_agents[GetCurrentPlayer()]);
  } else if (GetCurrentPlayer() == 2 * N_AGENTS_PER_PLAYER) {
    PlayGaiaSpawnerMove(move); 
  } else if (GetCurrentPlayer() == 2 * N_AGENTS_PER_PLAYER + 1) {
    PlayGaiaPlacerMove(move); 
  }
}

void Bomberland::TileIsFree(TilePosition pos) const {
  return m_board[pos.first][pos.second] == Tile.EMPTY;
}
