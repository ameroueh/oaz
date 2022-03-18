#ifndef OAZ_GAMES_BOMBERLAND_AGENT_HPP_
#define OAZ_GAMES_BOMBERLAND_AGENT_HPP_

namespace oaz::games::bomberland {
class Agent {
  public:
    Agent():
      m_position(Coordinates(0, 0)),
      m_invulnerable_until(0),
      m_n_bombs(3),
      m_health(3),
      m_blast_radius(3),
      m_id(0) {}

    explicit Agent(Coordinates position, std::size_t id):
      m_position(position),
      m_invulnerable_until(0),
      m_n_bombs(3),
      m_health(3),
      m_blast_radius(3),
      m_id(id) {}

    Coordinates GetPosition() const {
      return m_position;
    }
    void SetPosition(Coordinates position) {
      m_position = position;
    }
    bool IsDead() const { return m_health == 0; }
    std::size_t GetHealth() const { return m_health; }
    void DealDamage(size_t tick, size_t invulnerability_ticks) {
      if (!IsInvulnerable(tick, invulnerability_ticks)) {
        m_health = m_health > 0 ? m_health - 1 : 0;
	m_invulnerable_until = tick + invulnerability_ticks;
      }
    } 
    std::size_t GetNBombs() const { return m_n_bombs; }
    void RemoveBomb() {
      m_n_bombs = m_n_bombs > 0 ? m_n_bombs - 1 : 0;
    }
    void AddBomb() {
      ++m_n_bombs;
    }
    void IncreaseBlastRadius() {
      ++m_blast_radius;
    }
    std::size_t GetBlastRadius() const { return m_blast_radius; }
    std::size_t GetInvulnerableUntil() const {
      return m_invulnerable_until;
    }

    std::size_t GetId() const { return m_id; }
  private:
    bool IsInvulnerable(std::size_t tick, std::size_t invulnerability_ticks) const {
      return m_invulnerable_until > tick;
    }
    
    Coordinates m_position;
    std::size_t m_invulnerable_until;
    std::size_t m_n_bombs;
    std::size_t m_health;
    std::size_t m_blast_radius;
    std::size_t m_id;
};
} // namespace oaz::games::bomberland
#endif // OAZ_GAMES_BOMBERLAND_AGENT_HPP_
