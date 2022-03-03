#ifndef OAZ_GAMES_BOMBERLAND_COORDINATES_HPP_
#define OAZ_GAMES_BOMBERLAND_COORDINATES_HPP_

#include <cstddef>
#include <cstdint>

namespace oaz::games::bomberland {

class Coordinates {
  public:
  Coordinates(): m_first(0), m_second(0) {}
  Coordinates(int first, int second):
    m_first(first), m_second(second) {}
  int first() const { return m_first; }
  int second() const { return m_second; }
  Coordinates operator+(const Coordinates& p) {
    return Coordinates(first() + p.first(), second() + p.second());
  }
  void operator+=(const Coordinates& p) {
    *this = Coordinates(first() + p.first(), second() + p.second());
  }
  bool operator==(const Coordinates& p) const { return first() == p.first() && second() == p.second(); }


  private:
    signed int m_first : 5; 
    signed int m_second : 5; 
  
    /* friend class CoordinatesHasher; */
    /* friend class CoordinatesComparator; */
};

/* struct CoordinatesHasher { */
/*     std::size_t  operator() (const Coordinates& p) const { */
/*       return p.m_data; */
/*     } */
/* }; */

/* struct CoordinatesComparator { */
/*     bool operator()(const Coordinates& p1, const Coordinates& p2) const { */
/*       return p1 == p2; */
/*     } */
/* }; */

} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_COORDINATES_HPP_
