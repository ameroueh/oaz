#ifndef OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_
#define OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/agent_move_generator.hpp"

namespace oaz::games::bomberland {

class AgentMovePlayer {
  public:
    void operator()(
      size_t player,
      size_t agent_id,
      size_t packed_move,
      size_t tick,
      Board& board,
      PositionResolver& position_resolver) {
    MoveWithOperand move_with_op = AgentMoveUnpacker()(move);
    AgentMove move = move_with_op.first;
    Coordinates position = move_with_op.second;
    
    if (move == Pass) { return; }
    if (move == DetonateBomb) {
      Tile& tile = board.GetTile(position);
      if (!tile.HasPlacedBomb() || tile.GetOwner() != player) {
	return;
      }
      BombDetonator(position, board, tick);
    }
    Coordinates vector;
    switch (move) {
      case Up:
        vector = Coordinates(1, 0);
      case Down:
	vector = Coordinates(-1, 0);
      case Right:
	vector = Coordinates(0, 1);
      case Left:
	vector = Coordinates(0, -1);
    }
    Coordinates claimed_position = position + vector;
    if (!board.IsWithinBounds(claimed_position) || !board.GetTile(claimed_position).IsWalkable()) { return; }
    position_resolver.ClaimPosition(player, agent_id, claimed_position); 
  }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_AGENT_MOVE_PLAYER_HPP_
