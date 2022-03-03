#ifndef OAZ_GAMES_BOMBERLAND_DETONATION_ORDER_HPP_
#define OAZ_GAMES_BOMBERLAND_DETONATION_ORDER_HPP_

#include <cstddef>
#include <utility>

#include "oaz/games/bomberland/coordinates.hpp"

namespace oaz::games::bomberland {

using DetonationOrder = std::pair<Coordinates, std::size_t>;

} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_DETONATION_ORDER_HPP_
