#ifndef OAZ_GAMES_BOMBERLAND_AGENT_MOVE_GENERATOR_HPP_
#define OAZ_GAMES_BOMBERLAND_AGENT_MOVE_GENERATOR_HPP_

namespace oaz::games::bomberland {

enum AgentMove {
  Pass,
  Up,
  Down,
  Left,
  Right,
  PlaceBomb,
  DetonateBomb
};

using MoveWithOperand = std::pair<AgentMove, Coordinates>;

class AgentMovePacker {
  public:
    size_t operator()(MoveWithOperand move_with_op) {
      AgentMove move = move_with_op.first;
      Coordinates position = move_with_op.second;
      if (static_cast<int>(move) < 6) { return static_cast<std::size_t>(move); }
      size_t first = position.first();
      size_t second = position.second();
      return (first << 12) | (second << 4) | 6;
    }
};

class AgentMoveUnpacker {
  public:
    
    MoveWithOperand operator()(size_t packed_move) {
      AgentMove move = static_cast<AgentMove>(packed_move & 0b1111);    
      if (move != DetonateBomb) { return MoveWithOperand(move, Coordinates(0, 0)); }
      size_t second = (packed_move >> 4) & 0b11111111;
      size_t first = (packed_move >> 12) & 0b11111111;
      return MoveWithOperand(move, Coordinates(first, second));
    }
};

template <class BombIterator>
class AgentMoveGenerator {
  public:
    void operator()(size_t player, std::vector<std::size_t>& moves, BombIterator begin, BombIterator end) {
      moves.clear();
      for (size_t i=0; i!=6; ++i) {
	moves.push_back(
	  AgentMovePacker()(MoveWithOperand(static_cast<AgentMove>(i), Coordinates(0, 0)))
	);
      }
      for (auto it = begin; it != end; ++it) {
	moves.push_back(AgentMovePacker()(MoveWithOperand(DetonateBomb, *it)));
      }
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_AGENT_MOVE_GENERATOR_HPP_
