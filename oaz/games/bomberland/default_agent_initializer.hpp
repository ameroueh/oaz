#ifndef OAZ_GAMES_BOMBERLAND_DEFAULT_AGENT_INITIALIZER_HPP_
#define OAZ_GAMES_BOMBERLAND_DEFAULT_AGENT_INITIALIZER_HPP_

#include "boost/multi_array.hpp"

#include "oaz/games/bomberland/agent.hpp"
#include "oaz/games/bomberland/coordinates.hpp"

namespace oaz::games::bomberland {

class DefaultAgentInitializer {
  public:
    DefaultAgentInitializer() {}
    void operator()(boost::multi_array<Agent, 2>& agents) {
      for (int i=0; i!=3; ++i) { agents[0][i] = Agent(Coordinates(0, i)); }
      for (int i=0; i!=3; ++i) { agents[1][i] = Agent(Coordinates(14-i, 14)); }
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_DEFAULT_AGENT_INITIALIZER_HPP_
