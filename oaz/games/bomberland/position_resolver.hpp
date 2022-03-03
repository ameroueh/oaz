#ifndef OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_
#define OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_

#include "boost/multi_array.hpp"

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"

namespace oaz::games::bomberland {

class PositionResolver {
  public:
    PositionResolver():
      m_claimed_position(boost::extents[N_PLAYERS][N_AGENTS]) {}
    void ClaimPosition(size_t player, size_t agent_id, Coordinates position, Board& board) {
      m_claimed_position[player][agent_id] = position;
      board.GetTile(position).IncreaseNClaimants();
    }
    Coordinates ResolvePosition(size_t player, size_t agent_id, Coordinates current_position, Board& board) {
      Coordinates claimed_position = m_claimed_position[player][agent_id];
      if (board.GetTile(claimed_position).GetNClaimants() > 1) {
        return current_position;
      } else {
        return claimed_position;
      }
    }
    void ResetClaims(Board& board) {
      for (auto it = m_claimed_position.origin();
	   it != m_claimed_position.origin()+ m_claimed_position.num_elements(); 	    ++it) {
        board.GetTile(*it).ResetNClaimants();
      }
    }
  private:
    static constexpr size_t N_PLAYERS = 2;
    static constexpr size_t N_AGENTS = 3;
    boost::multi_array<Coordinates, 2> m_claimed_position;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_
