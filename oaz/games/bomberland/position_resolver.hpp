#ifndef OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_
#define OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_

#include "boost/multi_array.hpp"

#include "oaz/games/bomberland/board.hpp"
#include "oaz/games/bomberland/coordinates.hpp"
#include "oaz/games/bomberland/tile.hpp"
#include "oaz/games/bomberland/agent.hpp"

namespace oaz::games::bomberland {

class PositionResolver {
  public:
    PositionResolver():
      m_claimed_position(boost::extents[N_PLAYERS][N_AGENTS]) {}
    void ClaimPosition(std::size_t player, std::size_t agent_id, Coordinates position, Board& board) {
      m_claimed_position[player][agent_id] = position;
      board.GetTile(position).IncreaseNClaimants();
    }
    Coordinates ResolvePosition(std::size_t player, std::size_t agent_id, Coordinates current_position, Board& board) {
      Coordinates claimed_position = m_claimed_position[player][agent_id];
      if (board.GetTile(claimed_position).GetNClaimants() > 1) {
        return current_position;
      } else {
        return claimed_position;
      }
    }
    void AssignPositionsAndReset(boost::multi_array<Agent, 2>& agents, Board& board) {
      for (std::size_t player=0; player !=2; ++player) {
        for (std::size_t agent_id=0; agent_id != 3; ++agent_id) {
	  Agent& agent = agents[player][agent_id];
          agent.SetPosition(ResolvePosition(player, agent_id, agent.GetPosition(), board));
	}
      }
      ResetClaims(board);
    }
    void ResetClaims(Board& board) {
      for (auto it = m_claimed_position.origin();
	   it != m_claimed_position.origin()+ m_claimed_position.num_elements(); 	    ++it) {
        board.GetTile(*it).ResetNClaimants();
      }
    }
    Coordinates GetClaimedPosition(std::size_t player, std::size_t agent_id) const {
      return m_claimed_position[player][agent_id];
    }
  private:
    static constexpr std::size_t N_PLAYERS = 2;
    static constexpr std::size_t N_AGENTS = 3;
    boost::multi_array<Coordinates, 2> m_claimed_position;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_POSITION_RESOLVER_HPP_
