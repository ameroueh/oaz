#ifndef OAZ_GAMES_BOMBERLAND_BUILDER_FROM_JSON_HPP_
#define OAZ_GAMES_BOMBERLAND_BUILDER_FROM_JSON_HPP_

#include "oaz/games/bomberland/agent.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace oaz::games::bomberland {

EntityType GetEntityType(const json& type) {
  if (entity["type"] == "m") { return EntityType::MetalBlock; }
  if (entity["type"] == "w") { return EntityType::WoodenBlock; }
  if (entity["type"] == "o") { return EntityType::OreBlock; }
  if (entity["type"] == "a") { return EntityType::Ammunition; }
  if (entity["type"] == "bp") { return EntityType::BlastPowerup; }
  if (entity["type"] == "b") { return EntityType::Bomb; }
  if (entity["type"] == "x" && entity.contains("expires")) { return EntityType::Blast; }
  if (entity["type"] == "x" && !entity.contains("expires")) { return EntityType::Fire; }
 // To do: raise exception
}


class BuilderFromJson {
  public:
    BuilderFromJson() {}
    Bomberland operator()(const json& state) {
      Bomberland game;
      PopulateBoard(game, state["entities"]);
    }
  private:
    void PopulateBoard(Bomberland& game, const json& entities) {
      for (auto it = j.begin(); it !=j.end(); ++it) {
	Coordinates coords((*it)["x"], (*it)["y"]);
        Tile& tile = game.m_board.GetTile(coords);
        tile = CreateTileFromEntity(*it);
      }
    }
    Tile CreateTileFromEntity(const json& entity) {
      case (entity["type"]) {
        case 
      }
    }
    
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_BUILDER_FROM_JSON_HPP_
