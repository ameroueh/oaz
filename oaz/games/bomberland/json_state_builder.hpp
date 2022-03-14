#ifndef OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_
#define OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_

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

      return state;
    }
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_JSON_STATE_BUILDER_HPP_
