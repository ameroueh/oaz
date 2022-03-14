#ifndef OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_
#define OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/agent_move_generator.hpp"
#include "oaz/games/bomberland/event_manager.hpp"
#include "oaz/games/bomberland/position_resolver.hpp"

namespace oaz::games::bomberland {

class AgentMovePlayer {
  public:
    void operator()(
      size_t player,
      size_t agent_id,
      size_t packed_move,
      size_t tick,
      Board& board,
      boost::multi_array<Agent, 2>& agents,
      PositionResolver& position_resolver,
      EventManager& event_manager) {
    MoveWithOperand move_with_op = AgentMoveUnpacker()(packed_move);
    AgentMove move = move_with_op.first;
    Coordinates position = move_with_op.second;
    
    if (move == Pass) { return; }
    if (move == DetonateBomb) {
      Tile& tile = board.GetTile(position);
      if (!tile.HasPlacedBomb() || tile.GetOwner() != player) {
	return;
      }
      BombDetonator(position, board, event_manager, tick);
    }
    if (move == PlaceBomb) {
      Agent& agent = agents[player][agent_id];
      Tile& tile = board.GetTile(agent.GetPosition());
      if (!agent.GetNBombs() > 0 || !tile.IsEmptyTile()) {
	return;
      }
      tile = Tile::CreateTileWithPlacedBomb(player, tick, agent.GetBlastRadius());
      event_manager.AddEventFromTileAtPosition(agent.GetPosition(), board);
      agent.RemoveBomb();
      return;
    }
    Coordinates vector;
    switch (move) {
      case Up:
        vector = Coordinates(1, 0);
	break;
      case Down:
	vector = Coordinates(-1, 0);
	break;
      case Right:
	vector = Coordinates(0, 1);
	break;
      case Left:
	vector = Coordinates(0, -1);
	break;
    }
    Coordinates claimed_position = agents[player][agent_id].GetPosition() + vector;
    if (!board.IsWithinBounds(claimed_position) || !board.GetTile(claimed_position).IsWalkable()) { return; }
    position_resolver.ClaimPosition(player, agent_id, claimed_position, board); 
  }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_
