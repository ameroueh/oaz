#ifndef __GENERIC_GAME_MAP_HPP__
#define __GENERIC_GAME_MAP_HPP__

#include <map>

/* #include "boost/cast.hpp" */

#include "oaz/games/game.hpp"

namespace oaz::games {

template <class DerivedGame, class GameState>
class GenericGameMap : public Game::GameMap {
 public:
  bool Get(const Game& game, size_t* index) const override {
    const DerivedGame& derived_game = static_cast<const DerivedGame&>(game);

    auto iterator = m_map.find(derived_game.GetState());
    if (iterator == m_map.end()) {return false;}
    *index = iterator->second;
    return true;
  }

  void Insert(const Game& game, size_t index) override {
    const DerivedGame& derived_game = static_cast<const DerivedGame&>(game);

    m_map.emplace(std::pair<GameState, size_t>(derived_game.GetState(), index));
  }

  size_t GetSize() const override { return m_map.size(); }

 private:
  std::map<GameState, size_t> m_map;
};
}  // namespace oaz::games
#endif
