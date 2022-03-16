#ifndef OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_
#define OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_

#include "oaz/games/bomberland/agent.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace oaz::games::bomberland {

class JsonStateBuilder {
  public:
    JsonStateBuilder() {}
    nlohmann::json operator()(Bomberland& game) {
      json state;

      state["world"]["width"] = 15;
      state["world"]["height"] = 15;

      AddAgentsToState(game, state);
      AddEntitiesToState(game, state);

      state["tick"] = game.m_tick;

      return state;
    }
  private:
    void AddAgentsToState(Bomberland& game, json& state) {
      state["agents"]["a"] = { {"agent_id", "a"}, {"unit_ids", {"a", "b", "c"}}};
      state["agents"]["b"] = { {"agent_id", "b"}, {"unit_ids", {"d", "e", "f"}}};
      for (size_t player=0; player!=2; ++player)
	for (size_t agent_id=0; agent_id!=3; ++agent_id)
          state["unit_state"][std::string(1, 'a' + 3*player + agent_id)] = GetAgentStats(player, agent_id, game);
    }
    void AddEntitiesToState(Bomberland& game, json& state) {
      state["entities"] = json::array({});
      json& entities = state["entities"];
      for (size_t i=0; i!=15; ++i) {
	for (size_t j=0; j!=15; ++j) {
	  const Tile& tile = game.m_board.GetTile(Coordinates(i, j));
	  json entity;
	  switch (tile.GetEntityType()) {
	    case EntityType::None:
	      break;
	    case EntityType::Ammunition:
	      entity["type"] = "a";
	      entity["x"] = i;
	      entity["y"] = j;
	      entity["expires"] = tile.GetExpiryTime();
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::Bomb:
	      entity["type"] = "b";
	      entity["expires"] = tile.GetExpiryTime();
	      entity["x"] = i;
	      entity["y"] = j;
	      entity["blast_diameter"] = tile.GetBlastRadius();
	      entity["owner_unit_id"] = std::string(1, 'a' + tile.GetOwner());
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::Blast:
	      entity["type"] = "x";
	      entity["expires"] = tile.GetExpiryTime();
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::BlastPowerup:
	      entity["type"] = "bp";
	      entity["expires"] = tile.GetExpiryTime();
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::Fire:
	      entity["type"] = "x";
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::MetalBlock:
	      entity["type"] = "m";
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::OreBlock:
	      entity["type"] = "o";
	      entity["hp"] = tile.GetHP();
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	    case EntityType::WoodenBlock:
	      entity["type"] = "w";
	      entity["hp"] = tile.GetHP();
	      entity["x"] = i;
	      entity["y"] = j;
	      entities.insert(entities.begin(), entity);
	      break;
	  }
	}
      }
    }
    json GetAgentStats(size_t player, size_t agent_id, Bomberland& game) {
      auto agent = game.m_agents[player][agent_id];
      json stats;
      json coords = json::array({});
      coords.insert(coords.begin(), agent.GetPosition().first());
      coords.insert(coords.begin() + 1, agent.GetPosition().second());
      stats["coordinates"] = coords;
      stats["hp"] = agent.GetHealth();
      stats["inventory"]["bombs"] = agent.GetNBombs();
      stats["blast_diameter"] = agent.GetBlastRadius();
      stats["unit_id"] = std::string(1, 'a' + 3*player + agent_id);
      stats["owner_id"] = std::string(1, 'a' + player);
      stats["invulnerability"] = agent.GetInvulnerableUntil();
      return stats;
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_
